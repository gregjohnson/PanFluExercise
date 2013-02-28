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
        void addEvent(const int &nodeId, const StochasticSEATIRDEvent &event);

        // initiate transitions for an event according to a schedule
        void initializeExposedTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);
        void initializeAsymptomaticTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);
        void initializeTreatableTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);
        void initializeInfectiousTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);

        void initializeContactEvents(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);

        // process the next event
        bool nextEvent(int nodeId);

        // travel between nodes
        void travel();
};

#endif
