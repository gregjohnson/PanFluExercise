#include "StochasticSEATIRD.h"
#include "../../Parameters.h"
#include "../random.h"
#include "../../Stockpile.h"
#include "../../StockpileNetwork.h"
#include "../../log.h"
#include <boost/bind.hpp>

const int StochasticSEATIRD::numAgeGroups_ = 5;
const int StochasticSEATIRD::numRiskGroups_ = 2;
const int StochasticSEATIRD::numVaccinatedGroups_ = 2;

StochasticSEATIRD::StochasticSEATIRD()
{
    put_flog(LOG_DEBUG, "");

    // defaults
    cachedTime_ = -1;

    // create other required variables for this model
    newVariable("asymptomatic");
    newVariable("treatable");
    newVariable("infectious");
    newVariable("recovered");
    newVariable("deceased");

    // derived variables
    derivedVariables_[":infected"] = boost::bind(&StochasticSEATIRD::getInfected, this, _1, _2, _3);
    derivedVariables_[":hospitalized"] = boost::bind(&StochasticSEATIRD::getHospitalized, this, _1, _2, _3);

    // initialize start time to 0
    time_ = 0;
    now_ = 0.;

    // initiate random number generator
    gsl_rng_env_setup();
    randGenerator_ = gsl_rng_alloc(gsl_rng_default);
}

StochasticSEATIRD::~StochasticSEATIRD()
{
    put_flog(LOG_DEBUG, "");

    gsl_rng_free(randGenerator_);
}

int StochasticSEATIRD::expose(int num, int nodeId, std::vector<int> stratificationValues)
{
    // expose() can be called outside of a simulation before we've simulated any time steps
    if(cachedTime_ != time_)
    {
        put_flog(LOG_DEBUG, "precomputing (should only happen at the beginning of simulation)");

        precompute(time_);
    }

    int numExposed = EpidemicSimulation::expose(num, nodeId, stratificationValues);

    // create events based on these new exposures
    for(int i=0; i<numExposed; i++)
    {
        StochasticSEATIRDSchedule schedule(now_, rand_, stratificationValues);

        initializeContactEvents(schedule, nodeId, stratificationValues);

        // now add event schedules to big queue
        scheduleEventQueues_[nodeId].push(schedule);
    }

    return numExposed;
}

void StochasticSEATIRD::simulate()
{
    // we are simulating from time_ to time_+1
    now_ = (double)time_;

    // pre-compute some frequently used values
    precompute(time_);

    // base class simulate(): copies variables to new time step (time_+1) and evolves stockpile network
    EpidemicSimulation::simulate();

    // enable this for schedule verification (this is expensive!)
#if 0
    if(verifyScheduleCounts() != true)
    {
        put_flog(LOG_ERROR, "failed verification of schedule counts");
    }
#endif

    // apply treatments
    applyAntivirals();
    applyVaccines();

    // process events for each node
    for(unsigned int i=0; i<nodeIds_.size(); i++)
    {
        int nodeId = nodeIds_[i];

        while(scheduleEventQueues_[nodeId].empty() != true && scheduleEventQueues_[nodeId].top().getTopEvent().time < (double)time_+1.)
        {
            // pop the schedule off the schedule queue
            StochasticSEATIRDSchedule schedule = scheduleEventQueues_[nodeId].top();
            scheduleEventQueues_[nodeId].pop();

            // make sure schedule isn't empty or canceled (it could be canceled from applying treatments, for example)
            if(schedule.empty() != true && schedule.canceled() != true)
            {
                // pop the event off the schedule's event queue
                StochasticSEATIRDEvent event = schedule.getTopEvent();
                schedule.popTopEvent();

                // process the event
                now_ = event.time;

                processEvent(nodeId, event);

                // re-insert the schedule back into the schedule queue
                // it will be sorted corresponding to its next event
                if(schedule.empty() != true)
                {
                    scheduleEventQueues_[nodeId].push(schedule);
                }
            }
        }
    }

    // current event time is now the end of the current day
    now_ = (double)time_ + 1.;

    // travel between nodes
    travel();

    // increment current time
    time_++;
}

float StochasticSEATIRD::getInfected(int time, int nodeId, std::vector<int> stratificationValues)
{
    float infected = 0.;
    infected += getValue("asymptomatic", time, nodeId, stratificationValues);
    infected += getValue("treatable", time, nodeId, stratificationValues);
    infected += getValue("infectious", time, nodeId, stratificationValues);

    return infected;
}

float StochasticSEATIRD::getHospitalized(int time, int nodeId, std::vector<int> stratificationValues)
{
    float hospitalized = 0.;
    hospitalized += getValue("treatable", time, nodeId, stratificationValues);
    hospitalized += getValue("infectious", time, nodeId, stratificationValues);

    // todo: this is just an example...
    hospitalized *= 0.05;

    return hospitalized;
}

void StochasticSEATIRD::initializeContactEvents(StochasticSEATIRDSchedule &schedule, const int &nodeId, const std::vector<int> &stratificationValues)
{
    // todo: beta should be age-specific considering PHA's
    double beta = g_parameters.getR0() / g_parameters.getBetaScale();

    // todo: should be age-specific
    double vaccineEffectiveness = g_parameters.getVaccineEffectiveness();

    // todo: should be in parameters
    static double sigma[] = {1.00, 0.98, 0.94, 0.91, 0.66};

    // todo: should be in parameters
    static double contact[5][5] = {    { 45.1228487783,8.7808312353,11.7757947836,6.10114751268,4.02227175596 },
                                { 8.7808312353,41.2889143668,13.3332813497,7.847051289,4.22656343551 },
                                { 11.7757947836,13.3332813497,21.4270155984,13.7392636644,6.92483172729 },
                                { 6.10114751268,7.847051289,13.7392636644,18.0482119252,9.45371062356 },
                                { 4.02227175596,4.22656343551,6.92483172729,9.45371062356,14.0529294262 }   };

    // make sure we have expected stratifications
    if((int)stratifications_[0].size() != StochasticSEATIRD::numAgeGroups_ || (int)stratifications_[1].size() != StochasticSEATIRD::numRiskGroups_ || (int)stratifications_[2].size() != StochasticSEATIRD::numVaccinatedGroups_)
    {
        put_flog(LOG_ERROR, "wrong number of stratifications");
        return;
    }

    std::vector<int> toStratificationValues(3);

    for(int a=0; a<StochasticSEATIRD::numAgeGroups_; a++)
    {
        for(int r=0; r<StochasticSEATIRD::numRiskGroups_; r++)
        {
            for(int v=0; v<StochasticSEATIRD::numVaccinatedGroups_; v++)
            {
                //// this seems wrong -- the number of vaccinated people will change over time, and this targets them only at the time of exposure!

                toStratificationValues[0] = a;
                toStratificationValues[1] = r;
                toStratificationValues[2] = v;

                // fraction of the to group in population; use cached values
                double toGroupFraction = populations_(nodeIdToIndex_[nodeId], a, r, v) / populationNodes_(nodeIdToIndex_[nodeId]);

                double contactRate = contact[stratificationValues[0]][a];
                double transmissionRate = beta * contactRate * sigma[a] * toGroupFraction;

                // // vaccined stratification == 1
                if(v == 1)
                {
                    transmissionRate *= (1. - vaccineEffectiveness);
                }

                // contacts can occur within this time range
                double TcInit = schedule.getInfectedTMin(); // asymptomatic
                double TcFinal = schedule.getInfectedTMax(); // recovered / deceased

                // the first contact time...
                double Tc = TcInit + random_exponential(transmissionRate, &rand_);

                while(Tc < TcFinal)
                {
                    schedule.insertEvent(StochasticSEATIRDEvent(TcInit, Tc, CONTACT, stratificationValues, toStratificationValues));

                    TcInit = Tc;
                    Tc = TcInit + random_exponential(transmissionRate, &rand_);
                }
            }
        }
    }
}

bool StochasticSEATIRD::processEvent(const int &nodeId, const StochasticSEATIRDEvent &event)
{
    switch(event.type)
    {
        case EtoA:
            // exposed -> asymptomatic
            transition(1, "exposed", "asymptomatic", nodeId, event.fromStratificationValues);
            break;

        case AtoT:
            // asymptomatic -> treatable
            transition(1, "asymptomatic", "treatable", nodeId, event.fromStratificationValues);
            break;
        case AtoR:
            // asymptomatic -> recovered
            transition(1, "asymptomatic", "recovered", nodeId, event.fromStratificationValues);
            break;
        case AtoD:
            // asymptomatic -> deceased
            transition(1, "asymptomatic", "deceased", nodeId, event.fromStratificationValues);
            break;

        case TtoI:
            // treatable -> infectious
            transition(1, "treatable", "infectious", nodeId, event.fromStratificationValues);
            break;
        case TtoR:
            // treatable -> recovered
            transition(1, "treatable", "recovered", nodeId, event.fromStratificationValues);
            break;
        case TtoD:
            // treatable -> deceased
            transition(1, "treatable", "deceased", nodeId, event.fromStratificationValues);
            break;

        case ItoR:
            // infectious -> recovered
            transition(1, "infectious", "recovered", nodeId, event.fromStratificationValues);
            break;
        case ItoD:
            // infectious -> deceased
            transition(1, "infectious", "deceased", nodeId, event.fromStratificationValues);
            break;

        case CONTACT:
            int targetPopulationSize = (int)populations_(nodeIdToIndex_[nodeId], event.toStratificationValues[0], event.toStratificationValues[1], event.toStratificationValues[2]);

            if(event.fromStratificationValues == event.toStratificationValues)
            {
                targetPopulationSize -= 1; // - 1 because randint includes both endpoints
            }

            if(targetPopulationSize > 0)
            {
                // random integer between 1 and targetPopulationSize
                int contact = rand_.randInt(targetPopulationSize - 1) + 1;

                if((int)getValue("susceptible", time_+1, nodeId, event.toStratificationValues) >= contact)
                {
                    expose(1, nodeId, event.toStratificationValues);
                }
            }
    }

    return true;
}

void StochasticSEATIRD::applyAntivirals()
{
    // treatments for each node
    std::vector<int> nodeIds = getNodeIds();

    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        boost::shared_ptr<Stockpile> stockpile = getStockpileNetwork()->getNodeStockpile(nodeIds[i]);

        // do nothing if no stockpile is found
        if(stockpile == NULL)
        {
            continue;
        }

        // available antiviral stockpile
        int stockpileAmount = stockpile->getNum(time_+1, STOCKPILE_ANTIVIRALS);

        // do nothing if we have no available stockpile
        if(stockpileAmount == 0)
        {
            continue;
        }

        int totalNumberTreatable = (int)getValue("treatable", time_+1, nodeIds[i]);

        int stockpileAmountUsed = stockpileAmount;

        if(stockpileAmountUsed > totalNumberTreatable)
        {
            stockpileAmountUsed = totalNumberTreatable;
        }

        // decrement antivirals stockpile
        stockpile->setNum(time_+1, stockpileAmount - stockpileAmountUsed, STOCKPILE_ANTIVIRALS);

        // number successfully treated (includes effectiveness)
        int totalNumberTreated = (int)((float)stockpileAmountUsed * g_parameters.getAntiviralEffectiveness());

        // do nothing if nobody is treated
        if(totalNumberTreated <= 0)
        {
            continue;
        }

        // apply treatments pro-rata across all stratifications
        // todo: when we're doing these fractional divisions of int, we need to make sure we're not truncating off treatments

        blitz::Array<int, NUM_STRATIFICATION_DIMENSIONS> numberTreatable(StochasticSEATIRD::numAgeGroups_, StochasticSEATIRD::numRiskGroups_, StochasticSEATIRD::numVaccinatedGroups_);
        blitz::Array<int, NUM_STRATIFICATION_DIMENSIONS> numberTreated(StochasticSEATIRD::numAgeGroups_, StochasticSEATIRD::numRiskGroups_, StochasticSEATIRD::numVaccinatedGroups_);

        for(int a=0; a<StochasticSEATIRD::numAgeGroups_; a++)
        {
            for(int r=0; r<StochasticSEATIRD::numRiskGroups_; r++)
            {
                for(int v=0; v<StochasticSEATIRD::numVaccinatedGroups_; v++)
                {
                    std::vector<int> stratificationValues;
                    stratificationValues.push_back(a);
                    stratificationValues.push_back(r);
                    stratificationValues.push_back(v);

                    numberTreatable(a, r, v) = (int)getValue("treatable", time_+1, nodeIds[i], stratificationValues);

                    // pro-rata based on treatable population
                    numberTreated(a, r, v) = (int)((float)numberTreatable(a, r, v) / (float)totalNumberTreatable * (float)totalNumberTreated);

                    if(numberTreated(a, r, v) <= 0)
                    {
                        continue;
                    }

                    put_flog(LOG_DEBUG, "numberTreatable = %i, numberTreated = %i", numberTreatable(a, r, v), numberTreated(a, r, v));

                    // transition those treated from "treatable" to "recovered"
                    transition(numberTreated(a, r, v), "treatable", "recovered", nodeIds[i], stratificationValues);
                }
            }
        }

        // now, adjust schedules for individuals that were treated
        // this will stop their transitions to other states and also their contact events
        boost::heap::pairing_heap<StochasticSEATIRDSchedule, boost::heap::compare<StochasticSEATIRDSchedule::compareByNextEventTime> >::iterator begin = scheduleEventQueues_[nodeIds[i]].begin();
        boost::heap::pairing_heap<StochasticSEATIRDSchedule, boost::heap::compare<StochasticSEATIRDSchedule::compareByNextEventTime> >::iterator end = scheduleEventQueues_[nodeIds[i]].end();

        boost::heap::pairing_heap<StochasticSEATIRDSchedule, boost::heap::compare<StochasticSEATIRDSchedule::compareByNextEventTime> >::iterator it;

        for(it=begin; it!=end && blitz::sum(numberTreated) > 0; it++)
        {
            if((*it).getState() == T)
            {
                std::vector<int> stratificationValues = (*it).getStratificationValues();

                if(numberTreated(BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues)) > 0)
                {
                    if(rand_.rand() <= numberTreated(BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues)) / numberTreatable(BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues)))
                    {
                        // cancel the remaining schedule
                        (*boost::heap::pairing_heap<StochasticSEATIRDSchedule, boost::heap::compare<StochasticSEATIRDSchedule::compareByNextEventTime> >::s_handle_from_iterator(it)).cancel();

                        numberTreated(BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues))--;
                    }

                    numberTreatable(BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues))--;
                }
            }
        }
    }
}

void StochasticSEATIRD::applyVaccines()
{
    double vaccineAdherence = g_parameters.getVaccineAdherence();
    double vaccineCapacity = g_parameters.getVaccineCapacity();

    // treatments for each node
    std::vector<int> nodeIds = getNodeIds();

    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        boost::shared_ptr<Stockpile> stockpile = getStockpileNetwork()->getNodeStockpile(nodeIds[i]);

        // do nothing if no stockpile is found
        if(stockpile == NULL)
        {
            continue;
        }

        // available vaccines stockpile
        int stockpileAmount = stockpile->getNum(time_+1, STOCKPILE_VACCINES);

        // do nothing if we have no available stockpile
        if(stockpileAmount == 0)
        {
            continue;
        }

        // stratifications that we'll use...
        std::vector<int> stratificationValuesVaccinated(3, STRATIFICATIONS_ALL);
        stratificationValuesVaccinated[2] = 1; // vaccinated

        std::vector<int> stratificationValuesUnvaccinated(3, STRATIFICATIONS_ALL);
        stratificationValuesUnvaccinated[2] = 0; // unvaccinated

        // determine total number of adherent susceptible unvaccinated
        float totalPopulation = getValue("population", time_+1, nodeIds[i]);
        float totalVaccinatedPopulation = getValue("population", time_+1, nodeIds[i], stratificationValuesVaccinated);
        float totalUnvaccinatedPopulation = getValue("population", time_+1, nodeIds[i], stratificationValuesUnvaccinated);
        float totalSusceptibleUnvaccinated = getValue("susceptible", time_+1, nodeIds[i], stratificationValuesUnvaccinated);

        // == (adherent unvaccinated population) * (fraction of unvaccinated population that is susceptible)
        float totalAdherentSusceptibleUnvaccinated = (vaccineAdherence * totalPopulation - totalVaccinatedPopulation) * totalSusceptibleUnvaccinated / totalUnvaccinatedPopulation;

        // we will use all of our available stockpile (subject to capacity constraint) to treat the adherent susceptible unvaccinated population
        int stockpileAmountUsed = stockpileAmount;

        if(stockpileAmountUsed > (int)totalAdherentSusceptibleUnvaccinated)
        {
            stockpileAmountUsed = (int)totalAdherentSusceptibleUnvaccinated;
        }

        if(stockpileAmountUsed > (int)(vaccineCapacity * totalPopulation))
        {
            stockpileAmountUsed = (int)(vaccineCapacity * totalPopulation);
        }

        // do nothing if no stockpile is used
        if(stockpileAmountUsed <= 0)
        {
            continue;
        }

        // decrement vaccines stockpile
        stockpile->setNum(time_+1, stockpileAmount - stockpileAmountUsed, STOCKPILE_VACCINES);

        // apply vaccines pro-rata across all stratifications

        blitz::Array<float, NUM_STRATIFICATION_DIMENSIONS-1> adherentSusceptibleUnvaccinated(StochasticSEATIRD::numAgeGroups_, StochasticSEATIRD::numRiskGroups_);
        blitz::Array<int, NUM_STRATIFICATION_DIMENSIONS-1> numberVaccinated(StochasticSEATIRD::numAgeGroups_, StochasticSEATIRD::numRiskGroups_);

        for(int a=0; a<StochasticSEATIRD::numAgeGroups_; a++)
        {
            for(int r=0; r<StochasticSEATIRD::numRiskGroups_; r++)
            {
                std::vector<int> stratificationValues(3, STRATIFICATIONS_ALL);
                stratificationValues[0] = a;
                stratificationValues[1] = r;

                // determine number of adherent susceptible unvaccinated
                stratificationValues[2] = STRATIFICATIONS_ALL;
                float population = getValue("population", time_+1, nodeIds[i], stratificationValues);

                stratificationValues[2] = 1; // vaccinated
                float vaccinatedPopulation = getValue("population", time_+1, nodeIds[i], stratificationValues);

                stratificationValues[2] = 0; // unvaccinated
                float unvaccinatedPopulation = getValue("population", time_+1, nodeIds[i], stratificationValues);
                float susceptibleUnvaccinated = getValue("susceptible", time_+1, nodeIds[i], stratificationValues);

                // == (adherent unvaccinated population) * (fraction of unvaccinated population that is susceptible)
                adherentSusceptibleUnvaccinated(a, r) = (vaccineAdherence * population - vaccinatedPopulation) * susceptibleUnvaccinated / unvaccinatedPopulation;

                // pro-rata by adherent susceptible unvaccinated population
                numberVaccinated(a, r) = (int)(adherentSusceptibleUnvaccinated(a, r) / totalAdherentSusceptibleUnvaccinated * (float)stockpileAmountUsed);

                if(numberVaccinated(a, r) <= 0)
                {
                    continue;
                }

                put_flog(LOG_DEBUG, "adherentSusceptibleUnvaccinated = %f, numberVaccinated = %i", adherentSusceptibleUnvaccinated(a, r), numberVaccinated(a, r));

                // move individuals from susceptible unvaccinated to susceptible vaccinated
                variables_["susceptible"](time_+1, nodeIdToIndex_[nodeIds[i]], a, r, 0) -= numberVaccinated(a, r);
                variables_["susceptible"](time_+1, nodeIdToIndex_[nodeIds[i]], a, r, 1) += numberVaccinated(a, r);

                // need to also manipulate the total population variable: individuals are changing stratifications as well as state
                variables_["population"](time_+1, nodeIdToIndex_[nodeIds[i]], a, r, 0) -= numberVaccinated(a, r);
                variables_["population"](time_+1, nodeIdToIndex_[nodeIds[i]], a, r, 1) += numberVaccinated(a, r);
            }
        }

        // the sum over numberVaccinated should equal stockpileAmountUsed
        if(blitz::sum(numberVaccinated) != stockpileAmountUsed)
        {
            put_flog(LOG_WARN, "numberVaccinated != stockpileAmountUsed (%i != %i)", blitz::sum(numberVaccinated), stockpileAmountUsed);
        }

        // no need to adjust schedules since susceptible individuals are not scheduled yet
    }
}

void StochasticSEATIRD::travel()
{
    // todo: these should be parameters defined elsewhere
    double RHO = 0.39;

    double sigma[] = { 1.00, 0.98, 0.94, 0.91, 0.66 };

    double contact[5][5] = {    { 45.1228487783,8.7808312353,11.7757947836,6.10114751268,4.02227175596 },
                                { 8.7808312353,41.2889143668,13.3332813497,7.847051289,4.22656343551 },
                                { 11.7757947836,13.3332813497,21.4270155984,13.7392636644,6.92483172729 },
                                { 6.10114751268,7.847051289,13.7392636644,18.0482119252,9.45371062356 },
                                { 4.02227175596,4.22656343551,6.92483172729,9.45371062356,14.0529294262 }   };

    double vaccineEffectiveness = g_parameters.getVaccineEffectiveness();

    for(unsigned int sinkNodeIndex=0; sinkNodeIndex < nodeIds_.size(); sinkNodeIndex++)
    {
        int sinkNodeId = nodeIds_[sinkNodeIndex];

        double populationSink = populationNodes_(nodeIdToIndex_[sinkNodeId]);

        std::vector<double> unvaccinatedProbabilities(StochasticSEATIRD::numAgeGroups_, 0.0);

        std::vector<double> ageBasedFlowReductions (5, 1.0);
        ageBasedFlowReductions[0] = 10; // 0-4  year olds
        ageBasedFlowReductions[1] = 2;  // 5-24 year olds
        ageBasedFlowReductions[4] = 2;  // 65+  year olds

        for(unsigned int sourceNodeIndex=0; sourceNodeIndex < nodeIds_.size(); sourceNodeIndex++)
        {
            int sourceNodeId = nodeIds_[sourceNodeIndex];

            double populationSource = populationNodes_(nodeIdToIndex_[sourceNodeId]);

            // pre-compute some frequently needed quantities
            std::vector<double> asymptomatics(StochasticSEATIRD::numAgeGroups_);
            std::vector<double> transmittings(StochasticSEATIRD::numAgeGroups_);

            for(int age=0; age<StochasticSEATIRD::numAgeGroups_; age++)
            {
                asymptomatics[age] = getValue("asymptomatic", time_+1, sourceNodeId, std::vector<int>(1,age));
                transmittings[age] = asymptomatics[age] + getValue("treatable", time_+1, sourceNodeId, std::vector<int>(1,age)) + getValue("infectious", time_+1, sourceNodeId, std::vector<int>(1,age));
            }

            if(sinkNodeId != sourceNodeId)
            {
                // flow data
                float travelFractionIJ = getTravel(sinkNodeId, sourceNodeId);
                float travelFractionJI = getTravel(sourceNodeId, sinkNodeId);

                if(travelFractionIJ > 0. || travelFractionJI > 0.)
                {
                    for(int a=0; a<StochasticSEATIRD::numAgeGroups_; a++)
                    {
                        double numberOfInfectiousContactsIJ = 0.;
                        double numberOfInfectiousContactsJI = 0.;

                        // todo: beta should be age-specific considering PHA's
                        double beta = g_parameters.getR0() / g_parameters.getBetaScale();

                        for(int b=0; b<StochasticSEATIRD::numAgeGroups_; b++)
                        {
                            double asymptomatic = asymptomatics[b];

                            double transmitting = transmittings[b];

                            double contactRate = contact[a][b];

                            numberOfInfectiousContactsIJ += transmitting * beta * RHO * contactRate * sigma[a] / ageBasedFlowReductions[a];
                            numberOfInfectiousContactsJI += asymptomatic * beta * RHO * contactRate * sigma[a] / ageBasedFlowReductions[b];
                        }

                        unvaccinatedProbabilities[a] += travelFractionIJ * numberOfInfectiousContactsIJ / populationSource;
                        unvaccinatedProbabilities[a] += travelFractionJI * numberOfInfectiousContactsJI / populationSink;
                    }
                }
            }
        }

        for(int a=0; a<StochasticSEATIRD::numAgeGroups_; a++)
        {
            for(int r=0; r<StochasticSEATIRD::numRiskGroups_; r++)
            {
                for(int v=0; v<StochasticSEATIRD::numVaccinatedGroups_; v++)
                {
                    double probability = unvaccinatedProbabilities[a];

                    // vaccined stratification == 1
                    if(v == 1)
                    {
                        probability *= (1. - vaccineEffectiveness);
                    }

                    std::vector<int> stratificationValues;
                    stratificationValues.push_back(a);
                    stratificationValues.push_back(r);
                    stratificationValues.push_back(v);

                    int sinkNumSusceptible = (int)(variables_["susceptible"](time_+1, nodeIdToIndex_[sinkNodeId], a, r, v) + 0.5); // continuity correction

                    if(sinkNumSusceptible > 0)
                    {
                        int numberOfExposures = (int)gsl_ran_binomial(randGenerator_, probability, sinkNumSusceptible);

                        expose(numberOfExposures, sinkNodeId, stratificationValues);
                    }
                }
            }
        }
    }
}

void StochasticSEATIRD::precompute(int time)
{
    cachedTime_ = time;

    blitz::Array<double, 1> populationNodes(numNodes_); // [nodeIndex]

    blitz::TinyVector<int, 1+NUM_STRATIFICATION_DIMENSIONS> shape;
    shape(0) = numNodes_;
    shape(1) = StochasticSEATIRD::numAgeGroups_;
    shape(2) = StochasticSEATIRD::numRiskGroups_;
    shape(3) = StochasticSEATIRD::numVaccinatedGroups_;

    blitz::Array<double, 1+NUM_STRATIFICATION_DIMENSIONS> populations(shape); // [nodeIndex, a, r, v]

    for(unsigned int i=0; i<nodeIds_.size(); i++)
    {
        int nodeId = nodeIds_[i];

        populationNodes((int)i) = getValue("population", time, nodeId);

        for(int a=0; a<StochasticSEATIRD::numAgeGroups_; a++)
        {
            for(int r=0; r<StochasticSEATIRD::numRiskGroups_; r++)
            {
                for(int v=0; v<StochasticSEATIRD::numVaccinatedGroups_; v++)
                {
                    std::vector<int> stratificationValues;
                    stratificationValues.push_back(a);
                    stratificationValues.push_back(r);
                    stratificationValues.push_back(v);

                    populations((int)i, a, r, v) = getValue("population", time, nodeId, stratificationValues);
                }
            }
        }
    }

    populationNodes_.reference(populationNodes);
    populations_.reference(populations);
}

int StochasticSEATIRD::getScheduleCount(const int &nodeId, const StochasticSEATIRDScheduleState &state, const std::vector<int> &stratificationValues)
{
    int count = 0;

    boost::heap::pairing_heap<StochasticSEATIRDSchedule, boost::heap::compare<StochasticSEATIRDSchedule::compareByNextEventTime> >::iterator begin = scheduleEventQueues_[nodeId].begin();
    boost::heap::pairing_heap<StochasticSEATIRDSchedule, boost::heap::compare<StochasticSEATIRDSchedule::compareByNextEventTime> >::iterator end = scheduleEventQueues_[nodeId].end();

    boost::heap::pairing_heap<StochasticSEATIRDSchedule, boost::heap::compare<StochasticSEATIRDSchedule::compareByNextEventTime> >::iterator it;

    for(it=begin; it!=end; it++)
    {
        if((*it).canceled() != true && (*it).getState() == state && (*it).getStratificationValues() == stratificationValues)
        {
            count++;
        }
    }

    return count;
}

bool StochasticSEATIRD::verifyScheduleCounts()
{
    bool verified = true;

    std::vector<int> nodeIds = getNodeIds();

    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        for(int a=0; a<StochasticSEATIRD::numAgeGroups_; a++)
        {
            for(int r=0; r<StochasticSEATIRD::numRiskGroups_; r++)
            {
                for(int v=0; v<StochasticSEATIRD::numVaccinatedGroups_; v++)
                {
                    std::vector<int> stratificationValues;
                    stratificationValues.push_back(a);
                    stratificationValues.push_back(r);
                    stratificationValues.push_back(v);

                    // only verify exposed, asymptomatic, treatable, infectious, as these are the only states having events

                    int exposed = (int)getValue("exposed", time_+1, nodeIds[i], stratificationValues);
                    int exposedScheduled = getScheduleCount(nodeIds[i], E, stratificationValues);

                    int asymptomatic = (int)getValue("asymptomatic", time_+1, nodeIds[i], stratificationValues);
                    int asymptomaticScheduled = getScheduleCount(nodeIds[i], A, stratificationValues);

                    int treatable = (int)getValue("treatable", time_+1, nodeIds[i], stratificationValues);
                    int treatableScheduled = getScheduleCount(nodeIds[i], T, stratificationValues);

                    int infectious = (int)getValue("infectious", time_+1, nodeIds[i], stratificationValues);
                    int infectiousScheduled = getScheduleCount(nodeIds[i], I, stratificationValues);

                    if(exposed != exposedScheduled)
                    {
                        put_flog(LOG_ERROR, "exposed != exposedScheduled (%i != %i)", exposed, exposedScheduled);
                        verified = false;
                    }

                    if(asymptomatic != asymptomaticScheduled)
                    {
                        put_flog(LOG_ERROR, "asymptomatic != asymptomaticScheduled (%i != %i)", asymptomatic, asymptomaticScheduled);
                        verified = false;
                    }

                    if(treatable != treatableScheduled)
                    {
                        put_flog(LOG_ERROR, "treatable != treatableScheduled (%i != %i)", treatable, treatableScheduled);
                        verified = false;
                    }

                    if(infectious != infectiousScheduled)
                    {
                        put_flog(LOG_ERROR, "infectious != infectiousScheduled (%i != %i)", infectious, infectiousScheduled);
                        verified = false;
                    }
                }
            }
        }
    }

    return verified;
}
