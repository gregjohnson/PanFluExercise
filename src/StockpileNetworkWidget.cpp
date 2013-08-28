#include "StockpileNetworkWidget.h"
#include "EpidemicDataSet.h"
#include "EpidemicSimulation.h"
#include "StockpileNetwork.h"
#include "Stockpile.h"
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

    clearWidgets();

    // create new widgets if needed
    if(dataSet_ != NULL)
    {
        boost::shared_ptr<StockpileNetwork> stockpileNetwork = dataSet_->getStockpileNetwork();

        if(stockpileNetwork != NULL)
        {
            // nothing necessary
        }
    }
}

void StockpileNetworkWidget::setTime(int time)
{
    time_ = time;
}

void StockpileNetworkWidget::clearWidgets()
{
    // delete all existing widgets
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
