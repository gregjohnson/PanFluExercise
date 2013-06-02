#include "ParametersWidget.h"
#include "Parameters.h"
#include "log.h"

ParametersWidget::ParametersWidget()
{
    // initialize form layout
    QWidget * widget = new QWidget();
    formLayout_ = new QFormLayout();
    widget->setLayout(formLayout_);

    setWidgetResizable(true);
    setWidget(widget);

    // add widgets for all parameters
    constructParameterWidget("R0", "Basic reproduction number", g_parameters.getR0(), 0., WIDGET_MAX_VALUE, SLOT(setR0(double)));
    constructParameterWidget("betaScale", "Scaling factor for beta (transmission rate given contact). beta = R0 / betaScale", g_parameters.getBetaScale(), 0., WIDGET_MAX_VALUE, SLOT(setBetaScale(double)));
    constructParameterWidget("tau", "exposed -> asymptomatic transition rate", g_parameters.getTau(), 0., WIDGET_MAX_VALUE, SLOT(setTau(double)));
    constructParameterWidget("kappa", "asymptomatic -> treatable transition rate", g_parameters.getKappa(), 0., WIDGET_MAX_VALUE, SLOT(setKappa(double)));
    constructParameterWidget("chi", "time spent before progressing from treatable to infectious", g_parameters.getChi(), 0., WIDGET_MAX_VALUE, SLOT(setChi(double)));
    constructParameterWidget("gamma", "asymptomatic, treatable, or infectious -> recovered transition rate", g_parameters.getGamma(), 0., WIDGET_MAX_VALUE, SLOT(setGamma(double)));
    constructParameterWidget("nu", "asymptomatic, treatable, or infectious -> deceased transition rate", g_parameters.getNu(), 0., WIDGET_MAX_VALUE, SLOT(setNu(double)));
    constructParameterWidget("Antiviral effectiveness", "antiviral effectiveness", g_parameters.getAntiviralEffectiveness(), 0., 1., SLOT(setAntiviralEffectiveness(double)));
    constructParameterWidget("Vaccine effectiveness", "vaccine effectiveness", g_parameters.getVaccineEffectiveness(), 0., 1., SLOT(setVaccineEffectiveness(double)));
    constructParameterWidget("Vaccine adherence", "vaccine adherence", g_parameters.getVaccineAdherence(), 0., 1., SLOT(setVaccineAdherence(double)));
    constructParameterWidget("Vaccine capacity", "daily vaccine distribution capacity as a fraction of population", g_parameters.getVaccineCapacity(), 0., 1., SLOT(setVaccineCapacity(double)));
}

void ParametersWidget::constructParameterWidget(std::string label, std::string description, double value, double min, double max, const char * setSlot)
{
    QDoubleSpinBox * spinBox = new QDoubleSpinBox();

    spinBox->setToolTip(description.c_str());
    spinBox->setValue(value);
    spinBox->setRange(min, max);
    spinBox->setDecimals(WIDGET_NUM_DECIMALS);

    formLayout_->addRow(new QLabel(label.c_str()), spinBox);

    connect(spinBox, SIGNAL(valueChanged(double)), &g_parameters, setSlot);
}
