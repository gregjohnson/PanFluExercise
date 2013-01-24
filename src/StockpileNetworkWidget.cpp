#include "StockpileNetworkWidget.h"
#include "EpidemicDataSet.h"
#include "EpidemicSimulation.h"
#include "StockpileNetwork.h"
#include "Stockpile.h"
#include "StockpileWidget.h"
#include "log.h"

StockpileNetworkWidget::StockpileNetworkWidget(MainWindow * mainWindow)
{
    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));
}

StockpileNetworkWidget::~StockpileNetworkWidget()
{
    // delete all existing widgets
    for(unsigned int i=0; i<stockpileWidgets_.size(); i++)
    {
        delete stockpileWidgets_[i];
    }

    stockpileWidgets_.clear();
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
        // delete all existing widgets
        for(unsigned int i=0; i<stockpileWidgets_.size(); i++)
        {
            delete stockpileWidgets_[i];
        }

        stockpileWidgets_.clear();

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
