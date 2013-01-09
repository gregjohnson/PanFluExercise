#ifndef STOCHASTIC_SEATIRD_H
#define STOCHASTIC_SEATIRD_H

#include "../../EpidemicSimulation.h"
#include "StochasticSEATIRDEvent.h"
#include "StochasticSEATIRDSchedule.h"
#include <queue>
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

        // random number generators
        MTRand rand_;
        gsl_rng * randGenerator_;

        // current time
        double now_;

        // event queue for each nodeId
        std::map<int, std::priority_queue<StochasticSEATIRDEvent, std::vector<StochasticSEATIRDEvent>, StochasticSEATIRDEvent::compareByTime> > eventQueue_;

        // add an event to the queue
        void addEvent(int nodeId, StochasticSEATIRDEvent event);

        // initiate transitions for an event according to a schedule
        void initializeExposedTransitions(int nodeId, std::vector<int> stratificationValues, StochasticSEATIRDSchedule schedule);
        void initializeAsymptomaticTransitions(int nodeId, std::vector<int> stratificationValues, StochasticSEATIRDSchedule schedule);
        void initializeTreatableTransitions(int nodeId, std::vector<int> stratificationValues, StochasticSEATIRDSchedule schedule);
        void initializeInfectiousTransitions(int nodeId, std::vector<int> stratificationValues, StochasticSEATIRDSchedule schedule);

        void initializeContactEvents(int nodeId, std::vector<int> stratificationValues, StochasticSEATIRDSchedule schedule);

        // process the next event
        bool nextEvent(int nodeId);

        // travel between nodes
        void travel();
};

#endif
