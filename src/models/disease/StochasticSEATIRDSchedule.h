#ifndef STOCHASTIC_SEATIRD_SCHEDULE_H
#define STOCHASTIC_SEATIRD_SCHEDULE_H

#include "StochasticSEATIRDEvent.h"
#include "../MersenneTwister.h"
#include <boost/heap/pairing_heap.hpp>

// an individual corresponding to a schedule can be in any of these states
// susceptible is not included, since events start after exposure
// if these are modified, need to modify applyVaccines()!
enum StochasticSEATIRDScheduleState
{
    E,
    A,
    T,
    I,
    R,
    D
};

class StochasticSEATIRDSchedule
{
    public:

        StochasticSEATIRDSchedule(const double &now, MTRand &rand, const std::vector<int> &stratificationValues);

        void insertEvent(const StochasticSEATIRDEvent &event);

        // see if schedule is empty
        bool empty() const;

        // get the top event, but don't pop it off the queue
        StochasticSEATIRDEvent getTopEvent() const;

        // get and pop the top event
        // when an event is popped, it is assumed the person corresponding to this schedule transitions to the next state of the event (if applicable for event type)
        void popTopEvent();

        // get stratification of individual corresponding to this schedule
        std::vector<int> getStratificationValues() const;

        // get state of individual corresponding to this schedule
        StochasticSEATIRDScheduleState getState() const;

        // get infected time bounds
        double getInfectedTMin() const;
        double getInfectedTMax() const;

        // see if schedule is canceled
        bool canceled() const;

        // cancel schedule (e.g. by treatment)
        void cancel();

        // change stratifications (this is used when scheduled individuals are vaccinated)
        void changeStratificationValues(std::vector<int> stratificationValues);

        // todo: we could save the latest event time in this class to make the comparisons faster...
        class compareByNextEventTime
        {
            public:
                bool operator()(const StochasticSEATIRDSchedule * lhs, const StochasticSEATIRDSchedule * rhs) const
                {
                    return (lhs->eventQueue_.top().time > rhs->eventQueue_.top().time);
                }

                bool operator()(const StochasticSEATIRDSchedule &lhs, const StochasticSEATIRDSchedule &rhs) const
                {
                    return (lhs.eventQueue_.top().time > rhs.eventQueue_.top().time);
                }
        };

    private:

        boost::heap::pairing_heap<StochasticSEATIRDEvent, boost::heap::compare<StochasticSEATIRDEvent::compareByTime> > eventQueue_;

        // stratification of individual corresponding to schedule
        std::vector<int> stratificationValues_;

        // current state of individual corresponding to schedule
        StochasticSEATIRDScheduleState state_;

        // infected times: from asymptomatic to recovery / death
        // new events (such as contacts) only allowed within this range!
        double infectedTMin_;
        double infectedTMax_;

        // if the schedule is canceled no further events should be processed
        bool canceled_;
};

#endif
