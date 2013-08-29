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

    // maximum width to prevent left docks from expanding too wide
    setMaximumWidth(400);

    // add widgets for all parameters
    constructParameterWidget("R0", "Basic reproduction number", g_parameters.getR0(), 0., WIDGET_MAX_VALUE, SLOT(setR0(double)), 3);
    // hide betaScale parameter: constructParameterWidget("betaScale", "Scaling factor for beta (transmission rate given contact). beta = R0 / betaScale", g_parameters.getBetaScale(), 0., WIDGET_MAX_VALUE, SLOT(setBetaScale(double)));
    constructParameterWidget("Latency period", "Average latency period (days)", g_parameters.getTau(), 0.1, 10., SLOT(setTau(double)), 2);
    constructParameterWidget("Asymptomatic period", "Average asymptomatic infectious period (days)", g_parameters.getKappa(), 0.1, 10., SLOT(setKappa(double)), 2);
    // treatment period is currently fixed to 1 day (required by antiviral distribution implemention)
    // constructParameterWidget("Treatment window", "number of days treatable with antivirals", g_parameters.getChi(), 0., WIDGET_MAX_VALUE, SLOT(setChi(double)));
    constructParameterWidget("Infectious period", "Average total infectious period (days)", g_parameters.getGamma(), 0.1, 10., SLOT(setGamma(double)), 2);

    // stratified CFR
    std::vector<std::string> labels;
    labels.push_back("CFR (0-4 years)");
    labels.push_back("CFR (5-24 years)");
    labels.push_back("CFR (25-49 years)");
    labels.push_back("CFR (50-64 years)");
    labels.push_back("CFR (65+ years)");

    std::vector<double> values;

    for(unsigned int i=0; i<5; i++)
    {
        values.push_back(g_parameters.getNu(i));
    }

    constructStratifiedParameterWidget(labels, "Case fatality rate", values, 0.000001, 0.999, SLOT(setNu(double)), 6);

    constructParameterWidget("Antiviral effectiveness", "probability that an individual treated within the treatment window will recover", g_parameters.getAntiviralEffectiveness(), 0., 1., SLOT(setAntiviralEffectiveness(double)), 3);
    constructParameterWidget("Antiviral adherence", "proportion of individuals that will seek antiviral treatments", g_parameters.getAntiviralAdherence(), 0., 1., SLOT(setAntiviralAdherence(double)), 3);
    constructParameterWidget("Antiviral capacity", "daily antiviral distribution capacity as a fraction of population", g_parameters.getAntiviralCapacity(), 0., 1., SLOT(setAntiviralCapacity(double)), 3);
    constructParameterWidget("Vaccine effectiveness", "probability that a vaccinated individual will have immunity (after the vaccine effectiveness lag period)", g_parameters.getVaccineEffectiveness(), 0., 1., SLOT(setVaccineEffectiveness(double)), 3);
    constructParameterWidget("Vaccine effectiveness lag", "number of days until a vaccination reaches maximum effectiveness", g_parameters.getVaccineLatencyPeriod(), 0, 60, SLOT(setVaccineLatencyPeriod(int)));
    constructParameterWidget("Vaccine adherence", "proportion of individuals seeking vaccination", g_parameters.getVaccineAdherence(), 0., 1., SLOT(setVaccineAdherence(double)), 3);
    constructParameterWidget("Vaccine capacity", "daily vaccine distribution capacity as a fraction of population", g_parameters.getVaccineCapacity(), 0., 1., SLOT(setVaccineCapacity(double)), 3);
}

void ParametersWidget::constructParameterWidget(std::string label, std::string description, double value, double min, double max, const char * setSlot, int numDecimals)
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
    spinBox->setDecimals(numDecimals);

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

void ParametersWidget::constructStratifiedParameterWidget(std::vector<std::string> labels, std::string description, std::vector<double> values, double min, double max, const char * setSlot, int numDecimals)
{
    QGroupBox * groupBox = new QGroupBox();
    QVBoxLayout * layout = new QVBoxLayout();
    groupBox->setLayout(layout);

    // description
    QLabel * descriptionLabel = new QLabel(description.c_str());
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);

    // label and widget for each
    QWidget * widget = new QWidget();
    QFormLayout * formLayout = new QFormLayout();
    widget->setLayout(formLayout);

    for(unsigned int i=0; i<labels.size(); i++)
    {
        // the spin box
        QDoubleSpinBox * spinBox = new QDoubleSpinBox();

        spinBox->setToolTip(description.c_str());
        spinBox->setRange(min, max);
        spinBox->setDecimals(numDecimals);

        // setValue() should occur after the decimal precision has been set
        spinBox->setValue(values[i]);

        // set index property
        spinBox->setProperty("index", QVariant::fromValue(i));

        // make connection
        connect(spinBox, SIGNAL(valueChanged(double)), &g_parameters, setSlot);

        formLayout->addRow(labels[i].c_str(), spinBox);
    }

    layout->addWidget(widget);

    // add the widgets and make connections
    layout_->addWidget(groupBox);
}
