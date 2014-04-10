#include "Parameters.h"
#include "EpidemicDataSet.h"
#include "PriorityGroup.h"
#include "Npi.h"
#include "PriorityGroupSelections.h"
#include "log.h"
#include <QtXmlPatterns>

Parameters g_parameters;

Parameters::Parameters()
{
    // defaults
    R0_ = 1.2;
    betaScale_ = 65.;
    tau_ = 1. / 0.833333;
    kappa_ = 1. / 0.52631579;
    chi_ = 1.; // must be fixed at 1 with current antiviral distribution implementation!
    gamma_ = 1. / 0.243902439024;

    // previous default nu = 0.0001

    /* seasonal low-risk age-specific nus:
    nu ( 0, 0 ) = 2.23193e-05;
    nu ( 1, 0 ) = 4.09747056486e-05;
    nu ( 2, 0 ) = 8.37293183202e-05;
    nu ( 3, 0 ) = 6.18089564208e-05;
    nu ( 4, 0 ) = 8.97814893927e-06;
    */
    std::vector<double> nuSeasonalUntransformed;
    nuSeasonalUntransformed.push_back(2.23193e-05);
    nuSeasonalUntransformed.push_back(4.09747056486e-05);
    nuSeasonalUntransformed.push_back(8.37293183202e-05);
    nuSeasonalUntransformed.push_back(6.18089564208e-05);
    nuSeasonalUntransformed.push_back(8.97814893927e-06);

    // transform by CFR = 1 - exp(-nu / gamma) ... but note we have the inverse of gamma above
    // can get back to standard nu (rate) with: nu = -gamma * ln(1 - CFR). with transformations: nu = -1./gamma * ln(1 - CFR)
    for(unsigned int i=0; i<5; i++)
    {
        nu_.push_back(1 - exp(-nuSeasonalUntransformed[i] * gamma_));
    }

    antiviralEffectiveness_ = 0.15;
    antiviralAdherence_ = 0.8;
    antiviralCapacity_ = 0.001;
    vaccineEffectiveness_ = 0.8;
    vaccineLatencyPeriod_ = 14;
    vaccineAdherence_ = 0.8;
    vaccineCapacity_ = 0.001;
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

double Parameters::getNu(int index)
{
    if(index >= nu_.size())
    {
        put_flog(LOG_ERROR, "index %i out of bounds", index);
        return 0.;
    }

    return nu_[index];
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

std::vector<boost::shared_ptr<Npi> > Parameters::getNpis()
{
    return npis_;
}

boost::shared_ptr<PriorityGroupSelections> Parameters::getAntiviralPriorityGroupSelections()
{
    return antiviralPriorityGroupSelections_;
}

boost::shared_ptr<PriorityGroupSelections> Parameters::getVaccinePriorityGroupSelections()
{
    return vaccinePriorityGroupSelections_;
}

void Parameters::loadXmlData(const std::string &filename)
{
    QXmlQuery query;

    if(query.setFocus(QUrl(filename.c_str())) == false)
    {
        put_flog(LOG_ERROR, "failed to load %s", filename.c_str());
        QMessageBox::warning(NULL, "Error", "Could not load file", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    // temp values
    char string[1024];
    QString qstring;
    double value;

    // R0
    sprintf(string, "string(//R0/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setR0(value);
    }

    // latencyPeriodDays
    sprintf(string, "string(//latencyPeriodDays/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setTau(value);
    }

    // asymptomaticPeriodDays
    sprintf(string, "string(//asymptomaticPeriodDays/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setKappa(value);
    }

    // infectiousPeriodDays
    sprintf(string, "string(//infectiousPeriodDays/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setGamma(value);
    }

    // caseFatalityRates
    std::vector<double> values;

    sprintf(string, "string(//caseFatalityRates/@value0)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
        values.push_back(qstring.toDouble());

    sprintf(string, "string(//caseFatalityRates/@value1)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
        values.push_back(qstring.toDouble());

    sprintf(string, "string(//caseFatalityRates/@value2)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
        values.push_back(qstring.toDouble());

    sprintf(string, "string(//caseFatalityRates/@value3)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
        values.push_back(qstring.toDouble());

    sprintf(string, "string(//caseFatalityRates/@value4)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
        values.push_back(qstring.toDouble());

    if(values.size() > 0)
    {
        setNu(values);
    }

    // antiviralEffectiveness
    sprintf(string, "string(//antiviralEffectiveness/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setAntiviralEffectiveness(value);
    }

    // antiviralAdherence
    sprintf(string, "string(//antiviralAdherence/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setAntiviralAdherence(value);
    }

    // antiviralCapacity
    sprintf(string, "string(//antiviralCapacity/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setAntiviralCapacity(value);
    }

    // vaccineEffectiveness
    sprintf(string, "string(//vaccineEffectiveness/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setVaccineEffectiveness(value);
    }

    // vaccineEffectivenessLagDays
    sprintf(string, "string(//vaccineEffectivenessLagDays/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setVaccineLatencyPeriod(value);
    }

    // vaccineAdherence
    sprintf(string, "string(//vaccineAdherence/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setVaccineAdherence(value);
    }

    // vaccineCapacity
    sprintf(string, "string(//vaccineCapacity/@value)");
    query.setQuery(string);
    if(query.evaluateTo(&qstring) == true)
    {
        value = qstring.toDouble();
        setVaccineCapacity(value);
    }
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
    QObject * senderObject = sender();

    if(senderObject != 0)
    {
        int index = senderObject->property("index").value<int>();

        if(index >= nu_.size())
        {
            put_flog(LOG_ERROR, "index %i out of bounds", index);
            return;
        }

        nu_[index] = value;

        put_flog(LOG_DEBUG, "%i: %f", index, value);
    }
    else
    {
        put_flog(LOG_ERROR, "should not call this method directly");
    }
}

void Parameters::setNu(std::vector<double> values)
{
    if(values.size() != 5)
    {
        put_flog(LOG_ERROR, "expected vector of size 5, got size %i", values.size());
    }

    for(unsigned int i=0; i<5; i++)
    {
        put_flog(LOG_DEBUG, "value %i = %f", i, values[i]);
    }

    nu_ = values;
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

void Parameters::clearNpis()
{
    npis_.clear();
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
