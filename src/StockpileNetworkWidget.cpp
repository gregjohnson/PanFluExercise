#include "StockpileNetworkWidget.h"
#include "EpidemicDataSet.h"
#include "EpidemicSimulation.h"
#include "StockpileNetwork.h"
#include "Stockpile.h"
#include "StockpileWidget.h"
#include "StockpileNetworkDistributionWidget.h"
#include "log.h"

StockpileNetworkWidget::StockpileNetworkWidget(MainWindow * mainWindow)
{
    // defaults
    time_ = 0;

    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    // add distribution button
    QPushButton * addDistributionButton = new QPushButton("&Add Distribution");
    layout_.addWidget(addDistributionButton);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));

    connect(addDistributionButton, SIGNAL(clicked()), this, SLOT(addDistribution()));
}

StockpileNetworkWidget::~StockpileNetworkWidget()
{
    clearWidgets();
}

void StockpileNetworkWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    bool keepWidgets = false;

    // keep widgets if this is an evolution of an existing simulation
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        if(simulation->getNumTimes() > 1)
        {
            keepWidgets = true;
        }
    }

    if(keepWidgets == false)
    {
        clearWidgets();

        // create new widgets if needed
        if(simulation != NULL)
        {
            boost::shared_ptr<StockpileNetwork> stockpileNetwork = simulation->getStockpileNetwork();

            if(stockpileNetwork != NULL)
            {
                std::vector<boost::shared_ptr<Stockpile> > stockpiles = stockpileNetwork->getStockpiles();

                for(unsigned int i=0; i<stockpiles.size(); i++)
                {
                    StockpileWidget * stockpileWidget = new StockpileWidget(stockpiles[i]);

                    stockpileWidgets_.push_back(stockpileWidget);
                    layout_.addWidget(stockpileWidget);
                }
            }
        }
    }
}

void StockpileNetworkWidget::setTime(int time)
{
    time_ = time;

    for(unsigned int i=0; i<stockpileWidgets_.size(); i++)
    {
        stockpileWidgets_[i]->setTime(time);
    }
}

void StockpileNetworkWidget::clearWidgets()
{
    // delete all existing widgets
    for(unsigned int i=0; i<stockpileWidgets_.size(); i++)
    {
        delete stockpileWidgets_[i];
    }

    stockpileWidgets_.clear();

    for(unsigned int i=0; i<stockpileNetworkDistributionWidgets_.size(); i++)
    {
        delete stockpileNetworkDistributionWidgets_[i];
    }

    stockpileNetworkDistributionWidgets_.clear();
}

void StockpileNetworkWidget::addDistribution()
{
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        StockpileNetworkDistributionWidget * distributionWidget = new StockpileNetworkDistributionWidget(simulation);

        stockpileNetworkDistributionWidgets_.push_back(distributionWidget);
        layout_.insertWidget(1, distributionWidget);
    }
    else
    {
        put_flog(LOG_ERROR, "not a valid simulation");

        QMessageBox::warning(this, "Error", "Not a valid simulation.", QMessageBox::Ok, QMessageBox::Ok);
    }
}
