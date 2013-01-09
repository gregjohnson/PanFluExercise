#ifndef STOCHASTIC_SEATIRD_EVENT_H
#define STOCHASTIC_SEATIRD_EVENT_H

#include <vector>

enum StochasticSEATIRDEventType
{
    NONE,
    EtoA,
    AtoT,
    AtoR,
    AtoD,
    TtoI,
    TtoR,
    TtoD,
    ItoR,
    ItoD,
    CONTACT
};

struct StochasticSEATIRDEvent
{
    StochasticSEATIRDEvent();

    double initializationTime;
    double time;
    StochasticSEATIRDEventType type;
    std::vector<int> fromStratificationValues;
    std::vector<int> toStratificationValues;

    class compareByTime
    {
        public:
            bool operator()(const StochasticSEATIRDEvent * lhs, const StochasticSEATIRDEvent * rhs) const
            {
                return (lhs->time > rhs->time);
            }

            bool operator()(const StochasticSEATIRDEvent &lhs, const StochasticSEATIRDEvent &rhs) const
            {
                return (lhs.time > rhs.time);
            }
    };
};

#endif
