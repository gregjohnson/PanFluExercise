#ifndef EPIDEMIC_SIMULATION_H
#define EPIDEMIC_SIMULATION_H

#include "EpidemicDataSet.h"

class EpidemicSimulation : public EpidemicDataSet
{
    public:

        EpidemicSimulation();

        void evolve();

    private:

};

#endif
