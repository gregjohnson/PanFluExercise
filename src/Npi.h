#ifndef NPI_H
#define NPI_H

#include <string>
#include <vector>

class Npi
{
    public:

        Npi(std::string name, int duration, std::vector<double> ageEffectiveness, std::vector<int> nodeIds);

        std::string getName();
        int getDuration();
        std::vector<double> getAgeEffectiveness();
        std::vector<int> getNodeIds();

    private:

        std::string name_;
        int duration_;
        std::vector<double> ageEffectiveness_;
        std::vector<int> nodeIds_;
};

#endif
