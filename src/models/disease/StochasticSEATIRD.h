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

        // random number generators
        MTRand rand_;
        gsl_rng * randGenerator_;

        // current time
        int nowInt_;
        double now_;

        // event queue for each nodeId and for each (int)time
        std::map<int, std::map<int, boost::heap::pairing_heap<StochasticSEATIRDEvent, boost::heap::compare<StochasticSEATIRDEvent::compareByTime> > > > eventQueue_;

        // counters used to unqueue events
        std::map<std::string, blitz::Array<int, 1+NUM_STRATIFICATION_DIMENSIONS> > counters_;

        // cached values
        int cachedTime_;
        blitz::Array<double, 1> populationNodes_;
        blitz::Array<double, 1+NUM_STRATIFICATION_DIMENSIONS> populations_;
        std::map<std::string, blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> > cachedInitialVariables_;

        // add an event to the queue
        void addEvent(const int &nodeId, const StochasticSEATIRDEvent &event);

        // counter access / manipulation
        int getCount(const int &nodeId, const std::vector<int> &stratificationValues, const std::string &counterType);
        void incrementCounter(const int &nodeId, const std::vector<int> &stratificationValues, const std::string &counterType, const int &count);

        // determine whether or not to keep events (they may be unqueued)
        bool keepEvent(const int &nodeId, const StochasticSEATIRDEvent &event);
        bool keepContact(const int &nodeId, const StochasticSEATIRDEvent &event);

        // initiate transitions for an event according to a schedule
        void initializeExposedTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);
        void initializeAsymptomaticTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);
        void initializeTreatableTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);
        void initializeInfectiousTransitions(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);

        void initializeContactEvents(const int &nodeId, const std::vector<int> &stratificationValues, const StochasticSEATIRDSchedule &schedule);

        // process the next event
        bool nextEvent(int nodeId);

        // treatments
        void applyTreatments();

        // travel between nodes
        void travel();

        // precompute / cache values for each time step
        void precompute(int time);
};

#endif
