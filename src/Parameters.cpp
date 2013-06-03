#include "Parameters.h"
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
    antiviralCapacity_ = 0.1;
    vaccineEffectiveness_ = 0.8;
    vaccineAdherence_ = 0.8;
    vaccineCapacity_ = 0.1;
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

double Parameters::getVaccineAdherence()
{
    return vaccineAdherence_;
}

double Parameters::getVaccineCapacity()
{
    return vaccineCapacity_;
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
