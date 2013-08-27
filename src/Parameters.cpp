#include "Parameters.h"
#include "EpidemicDataSet.h"
#include "PriorityGroup.h"
#include "Npi.h"
#include "PriorityGroupSelections.h"
#include "log.h"

Parameters g_parameters;

Parameters::Parameters()
{
    // defaults
    R0_ = 1.2;
    betaScale_ = 65.;
    tau_ = 0.833333;
    kappa_ = 0.52631579;
    chi_ = 1.;
    gamma_ = 0.243902439024;
    nu_ = 0.0001;
    antiviralEffectiveness_ = 0.15;
    antiviralAdherence_ = 0.8;
    antiviralCapacity_ = 0.02;
    vaccineEffectiveness_ = 0.8;
    vaccineLatencyPeriod_ = 14;
    vaccineAdherence_ = 0.8;
    vaccineCapacity_ = 0.02;
}

double Parameters::getR0()
{
    return R0_;
}

double Parameters::getBetaScale()
{
    return betaScale_;
}

double Parameters::getTau()
{
    return tau_;
}

double Parameters::getKappa()
{
    return kappa_;
}

double Parameters::getChi()
{
    return chi_;
}

double Parameters::getGamma()
{
    return gamma_;
}

double Parameters::getNu()
{
    return nu_;
}

double Parameters::getAntiviralEffectiveness()
{
    return antiviralEffectiveness_;
}

double Parameters::getAntiviralAdherence()
{
    return antiviralAdherence_;
}

double Parameters::getAntiviralCapacity()
{
    return antiviralCapacity_;
}

double Parameters::getVaccineEffectiveness()
{
    return vaccineEffectiveness_;
}

int Parameters::getVaccineLatencyPeriod()
{
    return vaccineLatencyPeriod_;
}

double Parameters::getVaccineAdherence()
{
    return vaccineAdherence_;
}

double Parameters::getVaccineCapacity()
{
    return vaccineCapacity_;
}

std::vector<boost::shared_ptr<PriorityGroup> > Parameters::getPriorityGroups()
{
    return priorityGroups_;
}

boost::shared_ptr<PriorityGroupSelections> Parameters::getAntiviralPriorityGroupSelections()
{
    return antiviralPriorityGroupSelections_;
}

boost::shared_ptr<PriorityGroupSelections> Parameters::getVaccinePriorityGroupSelections()
{
    return vaccinePriorityGroupSelections_;
}

void Parameters::setR0(double value)
{
    R0_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setBetaScale(double value)
{
    betaScale_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setTau(double value)
{
    tau_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setKappa(double value)
{
    kappa_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setChi(double value)
{
    chi_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setGamma(double value)
{
    gamma_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setNu(double value)
{
    nu_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setAntiviralEffectiveness(double value)
{
    antiviralEffectiveness_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setAntiviralAdherence(double value)
{
    antiviralAdherence_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setAntiviralCapacity(double value)
{
    antiviralCapacity_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setVaccineEffectiveness(double value)
{
    vaccineEffectiveness_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setVaccineLatencyPeriod(int value)
{
    vaccineLatencyPeriod_ = value;

    put_flog(LOG_DEBUG, "%i", value);
}

void Parameters::setVaccineAdherence(double value)
{
    vaccineAdherence_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::setVaccineCapacity(double value)
{
    vaccineCapacity_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

void Parameters::addPriorityGroup(boost::shared_ptr<PriorityGroup> priorityGroup)
{
    priorityGroups_.push_back(priorityGroup);

    emit(priorityGroupAdded(priorityGroup));
}

void Parameters::addNpi(boost::shared_ptr<Npi> npi)
{
    npis_.push_back(npi);

    emit(npiAdded(npi));
}

void Parameters::setAntiviralPriorityGroupSelections(boost::shared_ptr<PriorityGroupSelections> priorityGroupSelections)
{
    antiviralPriorityGroupSelections_ = priorityGroupSelections;

    // log message
    std::string message;

    std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups = priorityGroupSelections->getPriorityGroups();

    for(unsigned int i=0; i<priorityGroups.size(); i++)
    {
        message += priorityGroups[i]->getName() + ", ";
    }

    put_flog(LOG_DEBUG, "%s", message.c_str());

    std::vector<std::vector<int> > stratificationValuesSet = priorityGroupSelections->getStratificationValuesSet();

    for(unsigned int i=0; i<stratificationValuesSet.size(); i++)
    {
        put_flog(LOG_DEBUG, "%i %i %i", stratificationValuesSet[i][0], stratificationValuesSet[i][1], stratificationValuesSet[i][2]);
    }
}

void Parameters::setVaccinePriorityGroupSelections(boost::shared_ptr<PriorityGroupSelections> priorityGroupSelections)
{
    vaccinePriorityGroupSelections_ = priorityGroupSelections;

    // log message
    std::string message;

    std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups = priorityGroupSelections->getPriorityGroups();

    for(unsigned int i=0; i<priorityGroups.size(); i++)
    {
        message += priorityGroups[i]->getName() + ", ";
    }

    put_flog(LOG_DEBUG, "%s", message.c_str());

    std::vector<std::vector<int> > stratificationValuesSet = priorityGroupSelections->getStratificationValuesSet2();

    for(unsigned int i=0; i<stratificationValuesSet.size(); i++)
    {
        put_flog(LOG_DEBUG, "%i %i %i", stratificationValuesSet[i][0], stratificationValuesSet[i][1], stratificationValuesSet[i][2]);
    }
}
