#include "ParametersWidget.h"
#include "Parameters.h"
#include "log.h"

ParametersWidget::ParametersWidget()
{
    // initialize form layout
    QWidget * widget = new QWidget();
    QFormLayout * layout = new QFormLayout();
    widget->setLayout(layout);

    setWidgetResizable(true);
    setWidget(widget);

    // add rows to the form layouts
    R0_spinBox_.setToolTip("Basic reproduction number");
    R0_spinBox_.setRange(0., WIDGET_MAX_VALUE);
    R0_spinBox_.setDecimals(WIDGET_NUM_DECIMALS);
    layout->addRow(new QLabel("R0"), &R0_spinBox_);
    connect(&R0_spinBox_, SIGNAL(valueChanged(double)), this, SLOT(setR0(double)));

    betaScale_spinBox_.setToolTip("Scaling factor for beta (transmission rate given contact). beta = R0 / betaScale");
    betaScale_spinBox_.setRange(0., WIDGET_MAX_VALUE);
    betaScale_spinBox_.setDecimals(WIDGET_NUM_DECIMALS);
    layout->addRow(new QLabel("betaScale"), &betaScale_spinBox_);
    connect(&betaScale_spinBox_, SIGNAL(valueChanged(double)), this, SLOT(setBetaScale(double)));

    tau_spinBox_.setToolTip("exposed -> asymptomatic transition rate");
    tau_spinBox_.setRange(0., WIDGET_MAX_VALUE);
    tau_spinBox_.setDecimals(WIDGET_NUM_DECIMALS);
    layout->addRow(new QLabel("tau"), &tau_spinBox_);
    connect(&tau_spinBox_, SIGNAL(valueChanged(double)), this, SLOT(setTau(double)));

    kappa_spinBox_.setToolTip("asymptomatic -> treatable infectious transition rate");
    kappa_spinBox_.setRange(0., WIDGET_MAX_VALUE);
    kappa_spinBox_.setDecimals(WIDGET_NUM_DECIMALS);
    layout->addRow(new QLabel("kappa"), &kappa_spinBox_);
    connect(&kappa_spinBox_, SIGNAL(valueChanged(double)), this, SLOT(setKappa(double)));

    chi_spinBox_.setToolTip("time spent before progressing from treatable to untreatable infectious");
    chi_spinBox_.setRange(0., WIDGET_MAX_VALUE);
    chi_spinBox_.setDecimals(WIDGET_NUM_DECIMALS);
    layout->addRow(new QLabel("chi"), &chi_spinBox_);
    connect(&chi_spinBox_, SIGNAL(valueChanged(double)), this, SLOT(setChi(double)));

    gamma_spinBox_.setToolTip("asymptomatic or infectious -> recovered transition rate");
    gamma_spinBox_.setRange(0., WIDGET_MAX_VALUE);
    gamma_spinBox_.setDecimals(WIDGET_NUM_DECIMALS);
    layout->addRow(new QLabel("gamma"), &gamma_spinBox_);
    connect(&gamma_spinBox_, SIGNAL(valueChanged(double)), this, SLOT(setGamma(double)));

    nu_spinBox_.setToolTip("asymptomatic or infectious -> deceased transition rate");
    nu_spinBox_.setRange(0., WIDGET_MAX_VALUE);
    nu_spinBox_.setDecimals(WIDGET_NUM_DECIMALS);
    layout->addRow(new QLabel("nu"), &nu_spinBox_);
    connect(&nu_spinBox_, SIGNAL(valueChanged(double)), this, SLOT(setNu(double)));

    // update widget values
    readValues();
}

void ParametersWidget::readValues()
{
    R0_spinBox_.setValue(g_parameters.getR0());
    betaScale_spinBox_.setValue(g_parameters.getBetaScale());
    tau_spinBox_.setValue(g_parameters.getTau());
    kappa_spinBox_.setValue(g_parameters.getKappa());
    chi_spinBox_.setValue(g_parameters.getChi());
    gamma_spinBox_.setValue(g_parameters.getGamma());
    nu_spinBox_.setValue(g_parameters.getNu());
}

void ParametersWidget::setR0(double value)
{
    g_parameters.setR0(value);
}

void ParametersWidget::setBetaScale(double value)
{
    g_parameters.setBetaScale(value);
}

void ParametersWidget::setTau(double value)
{
    g_parameters.setTau(value);
}

void ParametersWidget::setKappa(double value)
{
    g_parameters.setKappa(value);
}

void ParametersWidget::setChi(double value)
{
    g_parameters.setChi(value);
}

void ParametersWidget::setGamma(double value)
{
    g_parameters.setGamma(value);
}

void ParametersWidget::setNu(double value)
{
    g_parameters.setNu(value);
}
