#include "Npi.h"

MTRand Npi::rand_;

Npi::Npi(std::string name, int executionTime, int duration, std::vector<double> ageEffectiveness, std::vector<int> nodeIds)
{
    name_ = name;
    executionTime_ = executionTime;
    duration_ = duration;
    ageEffectiveness_= ageEffectiveness;
    nodeIds_ = nodeIds;
}

std::string Npi::getName()
{
    return name_;
}

int Npi::getExecutionTime()
{
    return executionTime_;
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

// static method
double Npi::getNpiEffectiveness(std::vector<boost::shared_ptr<Npi> > npis, int nodeId, int time, int ageI, int ageJ)
{
    std::vector<double> effectivenesses;

    for(unsigned int i=0; i<npis.size(); i++)
    {
        // if nodeId is covered by this Npi
        std::vector<int> nodeIds = npis[i]->getNodeIds();
        bool npiHasNodeId = (std::find(nodeIds.begin(), nodeIds.end(), nodeId) != nodeIds.end());

        // if the Npi is active during this time
        bool npiActive = (time >= npis[i]->getExecutionTime() && time < (npis[i]->getExecutionTime() + npis[i]->getDuration()));

        if(npiHasNodeId == true && npiActive == true)
        {
            std::vector<double> ageEffectiveness = npis[i]->getAgeEffectiveness();

            double probDeleteI = ageEffectiveness[ageI];
            double probDeleteJ = ageEffectiveness[ageJ];

            double probDeleteCombined = probDeleteI + probDeleteJ - probDeleteI*probDeleteJ;

            effectivenesses.push_back(probDeleteCombined);
        }
    }

    if(effectivenesses.size() == 0)
    {
        return 0.;
    }
    else if(effectivenesses.size() == 1)
    {
        return effectivenesses[0];
    }
    else
    {
        // we have multiple active Npis
        double probKeep = 1.;

        for(unsigned int i=0; i<effectivenesses.size(); i++)
        {
            probKeep *= (1. - effectivenesses[i]);
        }

        return (1. - probKeep);
    }
}

// static method
bool Npi::isNpiEffective(std::vector<boost::shared_ptr<Npi> > npis, int nodeId, int time, int ageI, int ageJ)
{
    double effectiveness = Npi::getNpiEffectiveness(npis, nodeId, time, ageI, ageJ);

    if(Npi::rand_.rand() <= effectiveness)
    {
        return true;
    }

    return false;
}
