#ifndef EPIDEMIC_SIMULATION_H
#define EPIDEMIC_SIMULATION_H

#include "EpidemicDataSet.h"

class EpidemicSimulation : public EpidemicDataSet
{
    public:

        EpidemicSimulation();

        // expose <num> people in <nodeId> from the subset <stratificationValues>; returns number of actually exposed people
        // this moves them from the susceptible variable to the exposed variable
        virtual int expose(int num, int nodeId, std::vector<int> stratificationValues);

        virtual void simulate();

    protected:

        int transition(int num, std::string sourceVarName, std::string destVarName, int nodeId, std::vector<int> stratificationValues);

};

#endif
