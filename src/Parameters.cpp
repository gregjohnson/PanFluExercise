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
}

void Parameters::setR0(double value)
{
    R0_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

double Parameters::getR0()
{
    return R0_;
}

void Parameters::setBetaScale(double value)
{
    betaScale_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

double Parameters::getBetaScale()
{
    return betaScale_;
}

void Parameters::setTau(double value)
{
    tau_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

double Parameters::getTau()
{
    return tau_;
}

void Parameters::setKappa(double value)
{
    kappa_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

double Parameters::getKappa()
{
    return kappa_;
}

void Parameters::setChi(double value)
{
    chi_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

double Parameters::getChi()
{
    return chi_;
}

void Parameters::setGamma(double value)
{
    gamma_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

double Parameters::getGamma()
{
    return gamma_;
}

void Parameters::setNu(double value)
{
    nu_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

double Parameters::getNu()
{
    return nu_;
}

void Parameters::setAntiviralEffectiveness(double value)
{
    antiviralEffectiveness_ = value;

    put_flog(LOG_DEBUG, "%f", value);
}

double Parameters::getAntiviralEffectiveness()
{
    return antiviralEffectiveness_;
}
