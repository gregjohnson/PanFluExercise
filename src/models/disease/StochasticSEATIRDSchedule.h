#ifndef STOCHASTIC_SEATIRD_SCHEDULE_H
#define STOCHASTIC_SEATIRD_SCHEDULE_H

#include "../MersenneTwister.h"
#include <vector>

struct StochasticSEATIRDSchedule
{
    StochasticSEATIRDSchedule();

    StochasticSEATIRDSchedule(const double now, MTRand &rand, std::vector<int> stratificationValues);

    double Ta;      // time to progress from exposed to asymptomatic
    double Tt;      // time to progress from asymptomatic to treatable
    double Ti;      // time to progress from treatable to infectious
    double Tr_a;    // time to recover from asymptomatic
    double Tr_ti;   // time to recover from treatable/infectious
    double Td_a;    // time to death from asymptomatic
    double Td_ti;   // time to death from treatable/infectious
    double Trd_ati; // time to recovery or death from asymptomatic, treatable, or infectious
};

#endif
