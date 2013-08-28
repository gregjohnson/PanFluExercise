#include "ParametersWidget.h"
#include "Parameters.h"
#include "log.h"

ParametersWidget::ParametersWidget()
{
    // initialize form layout
    QWidget * widget = new QWidget();
    layout_ = new QVBoxLayout();
    widget->setLayout(layout_);

    setWidgetResizable(true);
    setWidget(widget);

    // add widgets for all parameters
    constructParameterWidget("R0", "Basic reproduction number", g_parameters.getR0(), 0., WIDGET_MAX_VALUE, SLOT(setR0(double)));
    // hide betaScale parameter: constructParameterWidget("betaScale", "Scaling factor for beta (transmission rate given contact). beta = R0 / betaScale", g_parameters.getBetaScale(), 0., WIDGET_MAX_VALUE, SLOT(setBetaScale(double)));
    constructParameterWidget("tau", "exposed -> asymptomatic transition rate (determines average latency period)", g_parameters.getTau(), 0., WIDGET_MAX_VALUE, SLOT(setTau(double)));
    constructParameterWidget("kappa", "asymptomatic -> treatable transition rate (determines average asymptomatic infectious period)", g_parameters.getKappa(), 0., WIDGET_MAX_VALUE, SLOT(setKappa(double)));
    constructParameterWidget("Treatment window", "number of days treatable with antivirals", g_parameters.getChi(), 0., WIDGET_MAX_VALUE, SLOT(setChi(double)));
    constructParameterWidget("gamma", "asymptomatic, treatable, or infectious -> recovered transition rate (determines average infectious period)", g_parameters.getGamma(), 0., WIDGET_MAX_VALUE, SLOT(setGamma(double)));
    constructParameterWidget("nu", "asymptomatic, treatable, or infectious -> deceased transition rate (determines case fatality rates)", g_parameters.getNu(), 0., WIDGET_MAX_VALUE, SLOT(setNu(double)));
    constructParameterWidget("Antiviral effectiveness", "probability that an individual treated within the treatment window will recover", g_parameters.getAntiviralEffectiveness(), 0., 1., SLOT(setAntiviralEffectiveness(double)));
    constructParameterWidget("Antiviral adherence", "proportion of individuals that will seek antiviral treatments", g_parameters.getAntiviralAdherence(), 0., 1., SLOT(setAntiviralAdherence(double)));
    constructParameterWidget("Antiviral capacity", "daily antiviral distribution capacity as a fraction of population", g_parameters.getAntiviralCapacity(), 0., 1., SLOT(setAntiviralCapacity(double)));
    constructParameterWidget("Vaccine effectiveness", "probability that a vaccinated individual will have immunity (after the vaccine effectiveness lag period)", g_parameters.getVaccineEffectiveness(), 0., 1., SLOT(setVaccineEffectiveness(double)));
    constructParameterWidget("Vaccine effectiveness lag", "number of days until a vaccination reaches maximum effectiveness", g_parameters.getVaccineLatencyPeriod(), 0, 60, SLOT(setVaccineLatencyPeriod(int)));
    constructParameterWidget("Vaccine adherence", "proportion of individuals seeking vaccination", g_parameters.getVaccineAdherence(), 0., 1., SLOT(setVaccineAdherence(double)));
    constructParameterWidget("Vaccine capacity", "daily vaccine distribution capacity as a fraction of population", g_parameters.getVaccineCapacity(), 0., 1., SLOT(setVaccineCapacity(double)));
}

void ParametersWidget::constructParameterWidget(std::string label, std::string description, double value, double min, double max, const char * setSlot)
{
    QGroupBox * groupBox = new QGroupBox();
    QHBoxLayout * layout = new QHBoxLayout();
    groupBox->setLayout(layout);

    // label and description
    QWidget * labelWidget = new QWidget();
    QVBoxLayout * labelLayout = new QVBoxLayout();
    labelWidget->setLayout(labelLayout);

    labelLayout->addWidget(new QLabel(label.c_str()));

    QLabel * descriptionLabel = new QLabel(description.c_str());
    descriptionLabel->setWordWrap(true);
    labelLayout->addWidget(descriptionLabel);

    layout->addWidget(labelWidget);

    // the spin box
    QDoubleSpinBox * spinBox = new QDoubleSpinBox();

    spinBox->setToolTip(description.c_str());
    spinBox->setRange(min, max);
    spinBox->setDecimals(WIDGET_NUM_DECIMALS);

    // setValue() should occur after the decimal precision has been set
    spinBox->setValue(value);

    layout->addWidget(spinBox);

    // add the widgets and make connections
    layout_->addWidget(groupBox);

    connect(spinBox, SIGNAL(valueChanged(double)), &g_parameters, setSlot);
}

void ParametersWidget::constructParameterWidget(std::string label, std::string description, int value, int min, int max, const char * setSlot)
{
    QGroupBox * groupBox = new QGroupBox();
    QHBoxLayout * layout = new QHBoxLayout();
    groupBox->setLayout(layout);

    // label and description
    QWidget * labelWidget = new QWidget();
    QVBoxLayout * labelLayout = new QVBoxLayout();
    labelWidget->setLayout(labelLayout);

    labelLayout->addWidget(new QLabel(label.c_str()));

    QLabel * descriptionLabel = new QLabel(description.c_str());
    descriptionLabel->setWordWrap(true);
    labelLayout->addWidget(descriptionLabel);

    layout->addWidget(labelWidget);

    // the spin box
    QSpinBox * spinBox = new QSpinBox();

    spinBox->setToolTip(description.c_str());
    spinBox->setRange(min, max);
    spinBox->setValue(value);

    layout->addWidget(spinBox);

    // add the widgets and make connections
    layout_->addWidget(groupBox);

    connect(spinBox, SIGNAL(valueChanged(int)), &g_parameters, setSlot);
}
