#include "EpidemicInitialCasesWidget.h"
#include "EpidemicDataSet.h"
#include "EpidemicSimulation.h"
#include "EpidemicCasesWidget.h"
#include "log.h"

EpidemicInitialCasesWidget::EpidemicInitialCasesWidget(MainWindow * mainWindow)
{
    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    // add cases button
    QPushButton * addCasesButton = new QPushButton("&Add Cases");
    layout_.addWidget(addCasesButton);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect(addCasesButton, SIGNAL(clicked()), this, SLOT(addCases()));
}

EpidemicInitialCasesWidget::~EpidemicInitialCasesWidget()
{
    // delete all existing case widgets
    for(unsigned int i=0; i<casesWidgets_.size(); i++)
    {
        delete casesWidgets_[i];
    }

    casesWidgets_.clear();
}

void EpidemicInitialCasesWidget::applyCases()
{
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        if(simulation->getNumTimes() == 1)
        {
            for(unsigned int i=0; i<casesWidgets_.size(); i++)
            {
                EpidemicCases cases = casesWidgets_[i]->getCases();

                put_flog(LOG_DEBUG, "exposing %i people in %i", cases.num, cases.nodeId);

                simulation->expose(cases.num, cases.nodeId, cases.stratificationValues);
            }
        }
        else
        {
            put_flog(LOG_ERROR, "cannot add initial cases after initial time");
        }
    }
    else
    {
        put_flog(LOG_ERROR, "not a valid simulation");
    }
}

void EpidemicInitialCasesWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    bool keepCaseWidgets = false;

    // keep case widgets if this is an evolution of an existing simulation
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        if(simulation->getNumTimes() > 1)
        {
            keepCaseWidgets = true;
        }
    }

    if(keepCaseWidgets == false)
    {
        // delete all existing case widgets
        for(unsigned int i=0; i<casesWidgets_.size(); i++)
        {
            delete casesWidgets_[i];
        }

        casesWidgets_.clear();
    }
}

void EpidemicInitialCasesWidget::addCases()
{
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        if(simulation->getNumTimes() == 1)
        {
            EpidemicCasesWidget * casesWidget = new EpidemicCasesWidget(simulation);

            casesWidgets_.push_back(casesWidget);
            layout_.addWidget(casesWidget);
        }
        else
        {
            put_flog(LOG_ERROR, "cannot add initial cases after initial time");

            QMessageBox::warning(this, "Error", "Cannot add initial cases after initial time.", QMessageBox::Ok, QMessageBox::Ok);
        }
    }
    else
    {
        put_flog(LOG_ERROR, "not a valid simulation");

        QMessageBox::warning(this, "Error", "Not a valid simulation.", QMessageBox::Ok, QMessageBox::Ok);
    }
}
