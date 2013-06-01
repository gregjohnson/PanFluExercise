#ifndef STOCHASTIC_SEATIRD_H
#define STOCHASTIC_SEATIRD_H

#include "../../EpidemicSimulation.h"
#include "StochasticSEATIRDEvent.h"
#include "StochasticSEATIRDSchedule.h"
#include <boost/heap/pairing_heap.hpp>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

class StochasticSEATIRD : public EpidemicSimulation
{
    public:

        StochasticSEATIRD();
        ~StochasticSEATIRD();

        int expose(int num, int nodeId, std::vector<int> stratificationValues);

        void simulate();

        // derived variables
        float getInfected(int time, int nodeId, std::vector<int> stratificationValues=std::vector<int>());
        float getHospitalized(int time, int nodeId, std::vector<int> stratificationValues=std::vector<int>());

    private:

        // dimensions of stratifications
        static const int numAgeGroups_;
        static const int numRiskGroups_;
        static const int numVaccinatedGroups_;

        // random number generators
        MTRand rand_;
        gsl_rng * randGenerator_;

        // current time step
        int time_;

        // current time for processing new events / new exposures
        double now_;

        // schedule event queue for each nodeId
        std::map<int, boost::heap::pairing_heap<StochasticSEATIRDSchedule, boost::heap::compare<StochasticSEATIRDSchedule::compareByNextEventTime> > > scheduleEventQueues_;

        // cached values
        int cachedTime_;
        blitz::Array<double, 1> populationNodes_;
        blitz::Array<double, 1+NUM_STRATIFICATION_DIMENSIONS> populations_;

        // create contact events and insert them into the schedule
        void initializeContactEvents(StochasticSEATIRDSchedule &schedule, const int &nodeId, const std::vector<int> &stratificationValues);

        // process the next event
        bool processEvent(const int &nodeId, const StochasticSEATIRDEvent &event);

        // treatments
        void applyAntivirals();

        // travel between nodes
        void travel();

        // precompute / cache values for each time step
        void precompute(int time);

        // count number of active (not canceled) events in schedules corresponding to state and stratifications for nodeId
        int getScheduleCount(const int &nodeId, const StochasticSEATIRDScheduleState &state, const std::vector<int> &stratificationValues);

        // verify that the queued schedules match what is expected
        bool verifyScheduleCounts();
};

#endif
