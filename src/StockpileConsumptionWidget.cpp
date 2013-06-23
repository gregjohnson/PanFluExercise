#include "StockpileConsumptionWidget.h"
#include "PriorityGroupSelectionsWidget.h"
#include "Parameters.h"
#include "log.h"

StockpileConsumptionWidget::StockpileConsumptionWidget()
{
    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    antiviralPriorityGroupSelectionsWidget_ = new PriorityGroupSelectionsWidget("Antiviral Priority Groups");
    vaccinePriorityGroupSelectionsWidget_ = new PriorityGroupSelectionsWidget("Vaccine Priority Groups");

    layout_.addWidget(antiviralPriorityGroupSelectionsWidget_);
    layout_.addWidget(vaccinePriorityGroupSelectionsWidget_);

    // make connections
    connect(antiviralPriorityGroupSelectionsWidget_, SIGNAL(priorityGroupSelectionsChanged(boost::shared_ptr<PriorityGroupSelections>)), &g_parameters, SLOT(setAntiviralPriorityGroupSelections(boost::shared_ptr<PriorityGroupSelections>)));

    connect(vaccinePriorityGroupSelectionsWidget_, SIGNAL(priorityGroupSelectionsChanged(boost::shared_ptr<PriorityGroupSelections>)), &g_parameters, SLOT(setVaccinePriorityGroupSelections(boost::shared_ptr<PriorityGroupSelections>)));
}
