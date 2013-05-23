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
    StochasticSEATIRDEvent(const double &_initializationTime, const double &_time, const StochasticSEATIRDEventType &_type, const std::vector<int> &_fromStratificationValues, const std::vector<int> &_toStratificationValues)
    {
        initializationTime = _initializationTime;
        time = _time;
        type = _type;
        fromStratificationValues = _fromStratificationValues;
        toStratificationValues = _toStratificationValues;
    }

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
