#include "EpidemicInitialCasesWidget.h"
#include "EpidemicDataSet.h"
#include "EpidemicSimulation.h"
#include "EpidemicCasesWidget.h"
#include "log.h"
#include <QtXmlPatterns>

EpidemicInitialCasesWidget::EpidemicInitialCasesWidget(MainWindow * mainWindow)
{
    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    // clear cases button
    QPushButton * clearCasesButton = new QPushButton("&Clear Cases");
    layout_.addWidget(clearCasesButton);

    // add cases button
    QPushButton * addCasesButton = new QPushButton("&Add Cases");
    layout_.addWidget(addCasesButton);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect(clearCasesButton, SIGNAL(clicked()), this, SLOT(clearCases()));
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

                // disable the widget
                casesWidgets_[i]->setEnabled(false);
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

    // see if this is a simulation
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    // delete all existing case widgets
    for(unsigned int i=0; i<casesWidgets_.size(); i++)
    {
        delete casesWidgets_[i];
    }

    casesWidgets_.clear();

    // create defaults if this is a simulation
    if(simulation != NULL && simulation->getNumTimes() == 1)
    {
        // todo: this should be handled somewhere else
        int defaultNumCases = 10000;
        std::vector<int> defaultNodeIds;

        defaultNodeIds.push_back(453);
        defaultNodeIds.push_back(113);
        defaultNodeIds.push_back(201);
        defaultNodeIds.push_back(141);
        defaultNodeIds.push_back(375);

        for(unsigned int i=0; i<defaultNodeIds.size(); i++)
        {
            EpidemicCasesWidget * casesWidget = new EpidemicCasesWidget(simulation);

            casesWidgets_.push_back(casesWidget);
            layout_.addWidget(casesWidget);

            casesWidget->setNumCases(defaultNumCases);
            casesWidget->setNodeId(defaultNodeIds[i]);
        }
    }
}

void EpidemicInitialCasesWidget::loadXmlData(const std::string &filename)
{
    // see if this is a simulation
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation == NULL)
    {
        QMessageBox::warning(this, "Error", "No valid simulation.", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    else if(simulation->getNumTimes() != 1)
    {
        QMessageBox::warning(this, "Error", "Can only load initial cases at the beginning of the simulation.", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    // clear existing cases
    clearCases();

    QXmlQuery query;

    if(query.setFocus(QUrl(filename.c_str())) == false)
    {
        put_flog(LOG_ERROR, "failed to load %s", filename.c_str());
        QMessageBox::warning(this, "Error", "Could not load file", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    // temp strings
    char string[1024];
    QString qstring;

    // get number of initial cases
    sprintf(string, "string(count(//cases))");
    query.setQuery(string);
    query.evaluateTo(&qstring);
    int numCases = qstring.toInt();

    put_flog(LOG_INFO, "%i entries", numCases);

    // populate parameters for each tile
    for(int i=1; i<=numCases; i++)
    {
        sprintf(string, "string(//cases[%i]/@num)", i);
        query.setQuery(string);
        query.evaluateTo(&qstring);
        int num = qstring.toInt();

        sprintf(string, "string(//cases[%i]/@nodeId)", i);
        query.setQuery(string);
        query.evaluateTo(&qstring);
        int nodeId = qstring.toInt();

        put_flog(LOG_INFO, "%i cases for nodeId %i", num, nodeId);

        EpidemicCasesWidget * casesWidget = new EpidemicCasesWidget(simulation);

        casesWidgets_.push_back(casesWidget);
        layout_.addWidget(casesWidget);

        casesWidget->setNumCases(num);
        casesWidget->setNodeId(nodeId);
    }
}

void EpidemicInitialCasesWidget::clearCases()
{
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        if(simulation->getNumTimes() == 1)
        {
            // delete all existing case widgets
            for(unsigned int i=0; i<casesWidgets_.size(); i++)
            {
                delete casesWidgets_[i];
            }

            casesWidgets_.clear();
        }
        else
        {
            put_flog(LOG_ERROR, "cannot clear initial cases after initial time");

            QMessageBox::warning(this, "Error", "Cannot clear initial cases after initial time.", QMessageBox::Ok, QMessageBox::Ok);
        }
    }
    else
    {
        put_flog(LOG_ERROR, "not a valid simulation");

        QMessageBox::warning(this, "Error", "Not a valid simulation.", QMessageBox::Ok, QMessageBox::Ok);
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
