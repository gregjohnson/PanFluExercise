#include "StochasticSEATIRD.h"
#include "../../Parameters.h"
#include "../random.h"
#include "../../Stockpile.h"
#include "../../StockpileNetwork.h"
#include "../../log.h"
#include <boost/bind.hpp>

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

    // initial start time to 0
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
    if(cachedTime_ != numTimes_ - 1)
    {
        put_flog(LOG_DEBUG, "precomputing (should only happen at the beginning of simulation)");

        precompute(numTimes_ - 1);
    }

    int numExposed = EpidemicSimulation::expose(num, nodeId, stratificationValues);

    // create events based on these new exposures
    for(int i=0; i<numExposed; i++)
    {
        StochasticSEATIRDSchedule schedule(now_, rand_, stratificationValues);

        initializeExposedTransitions(nodeId, stratificationValues, schedule);
        initializeContactEvents(nodeId, stratificationValues, schedule);
    }

    return numExposed;
}

void StochasticSEATIRD::simulate()
{
    EpidemicSimulation::simulate();

    // pre-compute some frequently used values
    precompute(numTimes_ - 1);

    // apply treatments
    applyTreatments();

    double tMax = now_ + 1.0;

    // process events for each node
    for(unsigned int i=0; i<nodeIds_.size(); i++)
    {
        int nodeId = nodeIds_[i];

        while(!eventQueue_[nodeId][(int)now_].empty())
        {
            nextEvent(nodeId);
        }
    }

    // travel between nodes
    travel();

    now_ = tMax;
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

void StochasticSEATIRD::addEvent(const int &nodeId, const StochasticSEATIRDEvent &event)
{
    eventQueue_[nodeId][(int)event.time].push(event);

    if(event.type == CONTACT)
    {
        // increment contact counter
        incrementCounter(nodeId, event.fromStratificationValues, "queuedContact", 1);
    }
}

int StochasticSEATIRD::getCount(const int &nodeId, const std::vector<int> &stratificationValues, const std::string &counterType)
{
    // if we don't have a counter for this type, return 0
    if(counters_.count(counterType) == 0)
    {
        return 0;
    }

    return counters_[counterType](nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues));
}

void StochasticSEATIRD::incrementCounter(const int &nodeId, const std::vector<int> &stratificationValues, const std::string &counterType, const int &count)
{
    // if we don't have a counter for this type, create it
    if(counters_.count(counterType) == 0)
    {
        // shape
        blitz::TinyVector<int, 1+NUM_STRATIFICATION_DIMENSIONS> shape;

        shape(0) = numNodes_;

        for(int i=0; i<NUM_STRATIFICATION_DIMENSIONS; i++)
        {
            shape(1+i) = stratifications_[i].size();
        }

        // create the variable
        blitz::Array<int, 1+NUM_STRATIFICATION_DIMENSIONS> var(shape);

        // initialize values to zero
        var = 0;

        // add the variable to the vector
        counters_[counterType].reference(var);
    }

    // increment counter
    counters_[counterType](nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues)) += count;
}

bool StochasticSEATIRD::keepEvent(const int &nodeId, const StochasticSEATIRDEvent &event)
{
    bool keepEvent;

    std::string eventType;

    // right now only "treatable" and "infectious" events are unqueued here
    switch(event.type)
    {
        case TtoI:
        case TtoR:
        case TtoD:
            eventType = "treatable";
            break;

        case ItoR:
        case ItoD:
            eventType = "infectious";
            break;

        default:
            break;
    }

    if(eventType.empty() == true)
    {
        put_flog(LOG_ERROR, "unknown event type");
        return true;
    }

    if(counters_.count(eventType) == 0)
    {
        return true;
    }

    int unqueuedCount = counters_[eventType](nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, event.fromStratificationValues));

    if(eventType == "treatable" && (int)event.initializationTime == (int)now_)
    {
        keepEvent = true;
    }
    else
    {
        // todo: use of the cached initial variables is questionable...
        if(cachedInitialVariables_.count(eventType) == 0)
        {
            put_flog(LOG_ERROR, "no cached initial variable for %s", eventType.c_str());
            return true;
        }

        blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> initialVariable = cachedInitialVariables_[eventType];

        if(unqueuedCount == 0 || rand_.rand() > (float)unqueuedCount / ((float)unqueuedCount + initialVariable(nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, event.fromStratificationValues))))
        {
            keepEvent = true;
            initialVariable(nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, event.fromStratificationValues)) -= 1;
        }
        else
        {
            keepEvent = false;
            counters_[eventType](nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, event.fromStratificationValues)) -= 1;
        }
    }

    return keepEvent;
}

bool StochasticSEATIRD::keepContact(const int &nodeId, const StochasticSEATIRDEvent &event)
{
    bool keepEvent;

    if(counters_.count("unqueuedContact") == 0)
    {
        return true;
    }

    int unqueuedContactCount = counters_["unqueuedContact"](nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, event.fromStratificationValues));

    int queuedContactCount = counters_["queuedContact"](nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, event.fromStratificationValues));

    if(rand_.rand() > (double)unqueuedContactCount / (double)queuedContactCount)
    {
        keepEvent = true;
    }
    else
    {
        keepEvent = false;
        counters_["unqueuedContact"](nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, event.fromStratificationValues)) -= 1;
    }

    counters_["queuedContact"](nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, event.fromStratificationValues)) -= 1;

    return keepEvent;
}

void StochasticSEATIRD::initializeExposedTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    StochasticSEATIRDEvent event;

    event.initializationTime = now_;
    event.time = schedule.Ta;
    event.type = EtoA;
    event.fromStratificationValues = stratificationValues;
    event.toStratificationValues = stratificationValues;

    addEvent(nodeId, event);

    initializeAsymptomaticTransitions(nodeId, stratificationValues, schedule);
}

void StochasticSEATIRD::initializeAsymptomaticTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    StochasticSEATIRDEvent event;

    event.initializationTime = schedule.Ta;
    event.fromStratificationValues = stratificationValues;
    event.toStratificationValues = stratificationValues;

    // event time and type will vary...
    if(schedule.Tt < schedule.Tr_a && schedule.Tt < schedule.Td_a)
    {
        // asymptomatic -> treatable
        event.time = schedule.Tt;
        event.type = AtoT;

        initializeTreatableTransitions(nodeId, stratificationValues, schedule);
    }
    else if(schedule.Tr_a < schedule.Td_a)
    {
        // asymptomatic -> recovered
        event.time = schedule.Tr_a;
        event.type = AtoR;
    }
    else
    {
        // asymptomatic -> deceased
        event.time = schedule.Td_a;
        event.type = AtoD;
    }

    addEvent(nodeId, event);
}

void StochasticSEATIRD::initializeTreatableTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    StochasticSEATIRDEvent event;

    event.initializationTime = schedule.Tt;
    event.fromStratificationValues = stratificationValues;
    event.toStratificationValues = stratificationValues;

    // event time and type will vary...
    if(schedule.Ti < schedule.Tr_ti && schedule.Ti < schedule.Td_ti)
    {
        // treatable -> infectious
        event.time = schedule.Ti;
        event.type = TtoI;

        initializeInfectiousTransitions(nodeId, stratificationValues, schedule);
    }
    else if(schedule.Tr_ti < schedule.Td_ti)
    {
        // treatable -> recovered
        event.time = schedule.Tr_ti;
        event.type = TtoR;
    }
    else
    {
        // treatable -> deceased
        event.time = schedule.Td_ti;
        event.type = TtoD;
    }

    addEvent(nodeId, event);
}

void StochasticSEATIRD::initializeInfectiousTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    StochasticSEATIRDEvent event;

    event.initializationTime = schedule.Ti;
    event.fromStratificationValues = stratificationValues;
    event.toStratificationValues = stratificationValues;

    // event time and type will vary...
    if(schedule.Tr_ti < schedule.Td_ti)
    {
        // infectious -> recovered
        event.time = schedule.Tr_ti;
        event.type = ItoR;
    }
    else
    {
        // infectious -> deceased
        event.time = schedule.Td_ti;
        event.type = ItoD;
    }

    addEvent(nodeId, event);
}

void StochasticSEATIRD::initializeContactEvents(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule)
{
    // todo: beta should be age-specific considering PHA's
    double beta = g_parameters.getR0() / g_parameters.getBetaScale();

    // todo: need actual value here, should be age-specific
    static double vaccineEffectiveness = 0.;

    // todo: should be in parameters
    static double sigma[] = {1.00, 0.98, 0.94, 0.91, 0.66};

    // todo: should be in parameters
    static double contact[5][5] = {    { 45.1228487783,8.7808312353,11.7757947836,6.10114751268,4.02227175596 },
                                { 8.7808312353,41.2889143668,13.3332813497,7.847051289,4.22656343551 },
                                { 11.7757947836,13.3332813497,21.4270155984,13.7392636644,6.92483172729 },
                                { 6.10114751268,7.847051289,13.7392636644,18.0482119252,9.45371062356 },
                                { 4.02227175596,4.22656343551,6.92483172729,9.45371062356,14.0529294262 }   };

    // make sure we have expected stratifications
    // todo: make these defined elsewhere?
    static int numAgeGroups = 5;
    static int numRiskGroups = 2;
    static int numVaccinatedGroups = 2;

    if((int)stratifications_[0].size() != numAgeGroups || (int)stratifications_[1].size() != numRiskGroups || (int)stratifications_[2].size() != numVaccinatedGroups)
    {
        put_flog(LOG_ERROR, "wrong number of stratifications");
        return;
    }

    std::vector<int> toStratificationValues(3);

    for(int a=0; a<numAgeGroups; a++)
    {
        for(int r=0; r<numRiskGroups; r++)
        {
            for(int v=0; v<numVaccinatedGroups; v++)
            {
                toStratificationValues[0] = a;
                toStratificationValues[1] = r;
                toStratificationValues[2] = v;

                // fraction of the to group in population; this was cached before
                double toGroupFraction = populations_(nodeIdToIndex_[nodeId], a, r, v) / populationNodes_(nodeIdToIndex_[nodeId]);

                double contactRate = contact[stratificationValues[0]][a];
                double transmissionRate = (1. - vaccineEffectiveness) * beta * contactRate * sigma[a] * toGroupFraction;
                double TcInit = schedule.Ta;
                double Tc = random_exponential(transmissionRate, &rand_) + TcInit;

                while(Tc < schedule.Trd_ati)
                {
                    StochasticSEATIRDEvent event;

                    event.initializationTime = TcInit;
                    event.time = Tc;
                    event.type = CONTACT;
                    event.fromStratificationValues = stratificationValues;
                    event.toStratificationValues = toStratificationValues;

                    addEvent(nodeId, event);

                    TcInit = Tc;
                    Tc = TcInit + random_exponential(transmissionRate, &rand_);
                }
            }
        }
    }
}

bool StochasticSEATIRD::nextEvent(int nodeId)
{
    if(eventQueue_[nodeId][(int)now_].empty() == true)
    {
        return false;
    }

    // get and pop first event
    StochasticSEATIRDEvent event = eventQueue_[nodeId][(int)now_].top();
    eventQueue_[nodeId][(int)now_].pop();

    now_ = event.time;

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

        // todo: for TtoI, TtoR, TtoD look at keep_event, unqueue_event...
        case TtoI:
            // treatable -> infectious
            if(keepEvent(nodeId, event) == true)
            {
                transition(1, "treatable", "infectious", nodeId, event.fromStratificationValues);
            }
            else
            {
                // unqueue a subsequent infectious event
                incrementCounter(nodeId, event.fromStratificationValues, "infectious", 1);
            }

            break;
        case TtoR:
            // treatable -> recovered
            if(keepEvent(nodeId, event) == true)
            {
                transition(1, "treatable", "recovered", nodeId, event.fromStratificationValues);
            }

            break;
        case TtoD:
            // treatable -> deceased
            if(keepEvent(nodeId, event) == true)
            {
                transition(1, "treatable", "deceased", nodeId, event.fromStratificationValues);
            }

            break;

        // todo: for ItoR, ItoD look at keep_event
        case ItoR:
            // infectious -> recovered
            if(keepEvent(nodeId, event) == true)
            {
                transition(1, "infectious", "recovered", nodeId, event.fromStratificationValues);
            }

            break;
        case ItoD:
            // infectious -> deceased
            if(keepEvent(nodeId, event) == true)
            {
                transition(1, "infectious", "deceased", nodeId, event.fromStratificationValues);
            }

            break;

        case CONTACT:
            // todo: see if we keep this contact event
            if(keepContact(nodeId, event) == true)
            {
                // current time
                int time = numTimes_ - 1;

                int targetPopulationSize = (int)populations_(nodeIdToIndex_[nodeId], event.toStratificationValues[0], event.toStratificationValues[1], event.toStratificationValues[2]);

                if(event.fromStratificationValues == event.toStratificationValues)
                {
                    targetPopulationSize -= 1; // - 1 because randint includes both endpoints
                }

                if(targetPopulationSize > 0)
                {
                    // random integer between 1 and targetPopulationSize
                    int contact = rand_.randInt(targetPopulationSize - 1) + 1;

                    if((int)getValue("susceptible", time, nodeId, event.toStratificationValues) >= contact)
                    {
                        expose(1, nodeId, event.toStratificationValues);
                    }
                }
            }
    }

    return true;
}

void StochasticSEATIRD::applyTreatments()
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

        // available stockpile
        int stockpileAmount = stockpile->getNum((int)now_);

        // do nothing if we have no available stockpile
        if(stockpileAmount == 0)
        {
            continue;
        }

        float totalNumberTreatable = getValue("treatable", (int)now_, nodeIds[i]);

        int stockpileAmountUsed = stockpileAmount;

        if(stockpileAmountUsed > (int)totalNumberTreatable)
        {
            stockpileAmountUsed = (int)totalNumberTreatable;
        }

        // decrement stockpile
        stockpile->setNum((int)now_+1, stockpileAmount - stockpileAmountUsed);

        // number successfully treated (includes effectiveness)
        int totalNumberTreated = (int)((float)stockpileAmountUsed * g_parameters.getAntiviralEffectiveness());

        // do nothing if nobody is treated
        if(totalNumberTreated <= 0)
        {
            continue;
        }

        // apply treatments pro-rata across all stratifications
        // todo: when we're doing these fractional divisions of int, we need to make sure we're not truncating off treatments
        static int numAgeGroups = 5;
        static int numRiskGroups = 2;
        static int numVaccinatedGroups = 2;

        for(int a=0; a<numAgeGroups; a++)
        {
            for(int r=0; r<numRiskGroups; r++)
            {
                for(int v=0; v<numVaccinatedGroups; v++)
                {
                    std::vector<int> stratificationValues;
                    stratificationValues.push_back(a);
                    stratificationValues.push_back(r);
                    stratificationValues.push_back(v);

                    float numberTreatable = getValue("treatable", (int)now_, nodeIds[i], stratificationValues);

                    // pro-rata based on treatable population
                    int numberTreated = (int)(((float)numberTreatable / (float)totalNumberTreatable) * (float)totalNumberTreated);

                    if(numberTreated <= 0)
                    {
                        continue;
                    }

                    put_flog(LOG_DEBUG, "numberTreatable = %f, numTreated = %i", numberTreatable, numberTreated);

                    // transition those treated from "treatable" to "recovered"
                    transition(numberTreated, "treatable", "recovered", nodeIds[i], stratificationValues);

                    // unqueue treatable events via counter
                    incrementCounter(nodeIds[i], stratificationValues, "treatable", numberTreated);

                    // now, unqueue appropriate number of contact events
                    float probability = (float)numberTreated / getInfected((int)now_, nodeIds[i], stratificationValues);

                    int numTrials = getCount(nodeIds[i], stratificationValues, "queuedContact") - getCount(nodeIds[i], stratificationValues, "unqueuedContact");

                    int unqueuedContacts = 0;

                    if(probability >= 1.)
                    {
                        unqueuedContacts = numTrials;
                    }
                    else if(probability <= 0.)
                    {
                        unqueuedContacts = 0;
                    }
                    else
                    {
                        unqueuedContacts = gsl_ran_binomial(randGenerator_, probability, numTrials);
                    }

                    // unqueue contact events via counter
                    incrementCounter(nodeIds[i], stratificationValues, "unqueuedContact", unqueuedContacts);
                }
            }
        }
    }
}

void StochasticSEATIRD::travel()
{
    // current time
    int time = numTimes_ - 1;

    int numAgeGroups = 5;
    int numRiskGroups = 2;
    int numVaccinatedGroups = 2;

    // todo: these should be parameters defined elsewhere
    double RHO = 0.39;

    double sigma[] = { 1.00, 0.98, 0.94, 0.91, 0.66 };

    double contact[5][5] = {    { 45.1228487783,8.7808312353,11.7757947836,6.10114751268,4.02227175596 },
                                { 8.7808312353,41.2889143668,13.3332813497,7.847051289,4.22656343551 },
                                { 11.7757947836,13.3332813497,21.4270155984,13.7392636644,6.92483172729 },
                                { 6.10114751268,7.847051289,13.7392636644,18.0482119252,9.45371062356 },
                                { 4.02227175596,4.22656343551,6.92483172729,9.45371062356,14.0529294262 }   };

    double vaccineEffectiveness = 0.8;

    for(unsigned int sinkNodeIndex=0; sinkNodeIndex < nodeIds_.size(); sinkNodeIndex++)
    {
        int sinkNodeId = nodeIds_[sinkNodeIndex];

        double populationSink = populationNodes_(nodeIdToIndex_[sinkNodeId]);

        std::vector<double> unvaccinatedProbabilities(numAgeGroups, 0.0);

        std::vector<double> ageBasedFlowReductions (5, 1.0);
        ageBasedFlowReductions[0] = 10; // 0-4  year olds
        ageBasedFlowReductions[1] = 2;  // 5-24 year olds
        ageBasedFlowReductions[4] = 2;  // 65+  year olds

        for(unsigned int sourceNodeIndex=0; sourceNodeIndex < nodeIds_.size(); sourceNodeIndex++)
        {
            int sourceNodeId = nodeIds_[sourceNodeIndex];

            double populationSource = populationNodes_(nodeIdToIndex_[sourceNodeId]);

            // pre-compute some frequently needed quantities
            std::vector<double> asymptomatics(numAgeGroups);
            std::vector<double> transmittings(numAgeGroups);

            for(int age=0; age<numAgeGroups; age++)
            {
                asymptomatics[age] = getValue("asymptomatic", time, sourceNodeId, std::vector<int>(1,age));
                transmittings[age] = asymptomatics[age] + getValue("treatable", time, sourceNodeId, std::vector<int>(1,age)) + getValue("infectious", time, sourceNodeId, std::vector<int>(1,age));
            }

            if(sinkNodeId != sourceNodeId)
            {
                // flow data
                float travelFractionIJ = getTravel(sinkNodeId, sourceNodeId);
                float travelFractionJI = getTravel(sourceNodeId, sinkNodeId);

                if(travelFractionIJ > 0. || travelFractionJI > 0.)
                {
                    for(int a=0; a<numAgeGroups; a++)
                    {
                        double numberOfInfectiousContactsIJ = 0.;
                        double numberOfInfectiousContactsJI = 0.;

                        // todo: beta should be age-specific considering PHA's
                        double beta = g_parameters.getR0() / g_parameters.getBetaScale();

                        for(int b=0; b<numAgeGroups; b++)
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

        for(int a=0; a<numAgeGroups; a++)
        {
            for(int r=0; r<numRiskGroups; r++)
            {
                for(int v=0; v<numVaccinatedGroups; v++)
                {
                    double probability = 0.;

                    // vaccined stratification == 1
                    if(v == 1)
                    {
                        probability = (1. - vaccineEffectiveness) * unvaccinatedProbabilities[a];
                    }
                    else
                    {
                        probability = unvaccinatedProbabilities[a];
                    }

                    std::vector<int> stratificationValues;
                    stratificationValues.push_back(a);
                    stratificationValues.push_back(r);
                    stratificationValues.push_back(v);

                    int sinkNumSusceptible = (int)(variables_["susceptible"](time, nodeIdToIndex_[sinkNodeId], a, r, v) + 0.5); // continuity correction

                    double numberOfExposures = gsl_ran_binomial(randGenerator_, probability, sinkNumSusceptible);

                    expose(numberOfExposures, sinkNodeId, stratificationValues);
                }
            }
        }
    }
}

void StochasticSEATIRD::precompute(int time)
{
    cachedTime_ = time;

    int numAgeGroups = 5;
    int numRiskGroups = 2;
    int numVaccinatedGroups = 2;

    blitz::Array<double, 1> populationNodes(numNodes_); // [nodeIndex]

    blitz::TinyVector<int, 1+NUM_STRATIFICATION_DIMENSIONS> shape;
    shape(0) = numNodes_;
    shape(1) = numAgeGroups;
    shape(2) = numRiskGroups;
    shape(3) = numVaccinatedGroups;

    blitz::Array<double, 1+NUM_STRATIFICATION_DIMENSIONS> populations(shape); // [nodeIndex, a, r, v]

    for(unsigned int i=0; i<nodeIds_.size(); i++)
    {
        int nodeId = nodeIds_[i];

        populationNodes((int)i) = getValue("population", time, nodeId);

        for(int a=0; a<numAgeGroups; a++)
        {
            for(int r=0; r<numRiskGroups; r++)
            {
                for(int v=0; v<numVaccinatedGroups; v++)
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

    cachedInitialVariables_.clear();

    // only need to cache treatable and infectious variables
    std::vector<std::string> variables;

    variables.push_back("treatable");
    variables.push_back("infectious");

    for(unsigned int i=0; i<variables.size(); i++)
    {
        // this is a reference to the original data -- we don't want to modify it
        blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> var = getVariableAtFinalTime(variables[i]);

        // make a copy
        blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> varCopy = var.copy();

        cachedInitialVariables_[variables[i]].reference(varCopy);
    }
}
