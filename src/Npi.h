#ifndef NPI_H
#define NPI_H

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "models/MersenneTwister.h"

class Npi
{
    public:

        Npi(std::string name, int executionTime, int duration, std::vector<double> ageEffectiveness, std::vector<int> nodeIds);

        std::string getName();
        int getExecutionTime();
        int getDuration();
        std::vector<double> getAgeEffectiveness();
        std::vector<int> getNodeIds();

        // for the collection of Npis, at the given nodeId, time, two age groups: determine the effectiveness of all Npis combined
        static double getNpiEffectiveness(std::vector<boost::shared_ptr<Npi> > npis, int nodeId, int time, int ageI, int ageJ);

        // using the above, determine is all Npis combined are effective in stopping a contact
        static bool isNpiEffective(std::vector<boost::shared_ptr<Npi> > npis, int nodeId, int time, int ageI, int ageJ);

    private:

        std::string name_;
        int executionTime_;
        int duration_;
        std::vector<double> ageEffectiveness_;
        std::vector<int> nodeIds_;

        // random number generator
        static MTRand rand_;
};

#endif
