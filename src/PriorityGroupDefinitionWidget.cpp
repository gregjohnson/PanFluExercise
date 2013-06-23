#include "PriorityGroupDefinitionWidget.h"
#include "PriorityGroupWidget.h"
#include "Parameters.h"
#include "log.h"

PriorityGroupDefinitionWidget::PriorityGroupDefinitionWidget()
{
    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    // add priority group button
    QPushButton * addPriorityGroupButton = new QPushButton("&Add Priority Group");
    layout_.addWidget(addPriorityGroupButton);

    // make connections
    connect(addPriorityGroupButton, SIGNAL(clicked()), this, SLOT(addPriorityGroup()));

    updatePriorityGroups();
}

void PriorityGroupDefinitionWidget::clearWidgets()
{
    // delete all existing widgets
    for(unsigned int i=0; i<priorityGroupWidgets_.size(); i++)
    {
        delete priorityGroupWidgets_[i];
    }

    priorityGroupWidgets_.clear();
}

void PriorityGroupDefinitionWidget::updatePriorityGroups()
{
    clearWidgets();

    // create new widgets if needed
    // these would be defaults
    std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups = g_parameters.getPriorityGroups();

    for(unsigned int i=0; i<priorityGroups.size(); i++)
    {
        PriorityGroupWidget * priorityGroupWidget = new PriorityGroupWidget(priorityGroups[i]);

        priorityGroupWidgets_.push_back(priorityGroupWidget);
        layout_.insertWidget(1, priorityGroupWidget);
    }
}

void PriorityGroupDefinitionWidget::addPriorityGroup()
{
    PriorityGroupWidget * priorityGroupWidget = new PriorityGroupWidget();

    priorityGroupWidgets_.push_back(priorityGroupWidget);
    layout_.insertWidget(1, priorityGroupWidget);
}
