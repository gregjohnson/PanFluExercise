#include "PriorityGroupDefinitionWidget.h"
#include "MainWindow.h"
#include "PriorityGroupWidget.h"
#include "EpidemicSimulation.h"
#include "log.h"

PriorityGroupDefinitionWidget::PriorityGroupDefinitionWidget(MainWindow * mainWindow)
{
    // defaults
    time_ = 0;

    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    // add priority group button
    QPushButton * addPriorityGroupButton = new QPushButton("&Add Priority Group");
    layout_.addWidget(addPriorityGroupButton);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));

    connect(addPriorityGroupButton, SIGNAL(clicked()), this, SLOT(addPriorityGroup()));
}

void PriorityGroupDefinitionWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    clearWidgets();

    // create new widgets if needed
    // these would be defaults created with the simulation
    if(dataSet_ != NULL)
    {
        std::vector<PriorityGroup> priorityGroups = dataSet_->getPriorityGroups();

        for(unsigned int i=0; i<priorityGroups.size(); i++)
        {
            PriorityGroupWidget * priorityGroupWidget = new PriorityGroupWidget(dataSet_, priorityGroups[i]);

            priorityGroupWidgets_.push_back(priorityGroupWidget);
            layout_.insertWidget(1, priorityGroupWidget);
        }
    }
}

void PriorityGroupDefinitionWidget::setTime(int time)
{
    time_ = time;

    for(unsigned int i=0; i<priorityGroupWidgets_.size(); i++)
    {
        // todo: not currently used
    }
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

void PriorityGroupDefinitionWidget::addPriorityGroup()
{
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        PriorityGroupWidget * priorityGroupWidget = new PriorityGroupWidget(simulation);

        priorityGroupWidgets_.push_back(priorityGroupWidget);
        layout_.insertWidget(1, priorityGroupWidget);
    }
    else
    {
        put_flog(LOG_ERROR, "not a valid simulation");

        QMessageBox::warning(this, "Error", "Not a valid simulation.", QMessageBox::Ok, QMessageBox::Ok);
    }
}
