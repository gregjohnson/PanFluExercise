#include "Npi.h"

Npi::Npi(std::string name, int duration, std::vector<double> ageEffectiveness, std::vector<int> nodeIds)
{
    name_ = name;
    duration_ = duration;
    ageEffectiveness_= ageEffectiveness;
    nodeIds_ = nodeIds;
}

std::string Npi::getName()
{
    return name_;
}

int Npi::getDuration()
{
    return duration_;
}

std::vector<double> Npi::getAgeEffectiveness()
{
    return ageEffectiveness_;
}

std::vector<int> Npi::getNodeIds()
{
    return nodeIds_;
}
