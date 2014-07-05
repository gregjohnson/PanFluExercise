#include "StockpileNetworkWidget.h"
#include "EpidemicDataSet.h"
#include "EpidemicSimulation.h"
#include "StockpileNetwork.h"
#include "Stockpile.h"
#include "StockpileNetworkDistributionWidget.h"
#include "log.h"
#include <fstream>
#include <boost/tokenizer.hpp>

StockpileNetworkWidget::StockpileNetworkWidget(MainWindow * mainWindow)
{
    // defaults
    time_ = 0;

    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    // import antiviral distributions button
    QPushButton * importAntiviralDistributionsButton = new QPushButton("&Import Antiviral Distributions");
    layout_.addWidget(importAntiviralDistributionsButton);

    // add distribution button
    QPushButton * addDistributionButton = new QPushButton("&Add Distribution");
    layout_.addWidget(addDistributionButton);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));

    connect(importAntiviralDistributionsButton, SIGNAL(clicked()), this, SLOT(importAntiviralDistributions()));

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

void StockpileNetworkWidget::importAntiviralDistributions()
{
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation == NULL)
    {
        put_flog(LOG_ERROR, "not a valid simulation");
        QMessageBox::warning(this, "Error", "Not a valid simulation.", QMessageBox::Ok, QMessageBox::Ok);

        return;
    }

    QString filename = QFileDialog::getOpenFileName(this, "Import Antiviral Distributions", "", "CSV files (*.csv)");

    if(filename.isEmpty())
    {
        return;
    }

    std::ifstream in(filename.toStdString());

    if(in.is_open() != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", filename.toStdString().c_str());
        return;
    }

    // use boost tokenizer to parse the file
    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

    std::vector<std::string> vec;
    std::string line;

    // read (and ignore) header
    getline(in, line);

    while(getline(in, line))
    {
        Tokenizer tok(line);

        vec.assign(tok.begin(), tok.end());

        // each line is: Time Period (days),County FIPS,County Name,Antiviral Release Amount
        if(vec.size() != 4)
        {
            put_flog(LOG_ERROR, "number of values != 4, == %i", vec.size());
            return;
        }

        int time = atoi(vec[0].c_str());
        int nodeId = atoi(vec[1].c_str());
        int releaseAmount = atoi(vec[3].c_str());

        // add the distribution widget with given parameters and commit it
        StockpileNetworkDistributionWidget * distributionWidget = new StockpileNetworkDistributionWidget(simulation);

        if(nodeId == 0)
        {
            // new inventory to central
            distributionWidget->setExecutionTime(time);
            distributionWidget->setSource("New Inventory");
            distributionWidget->setDestination("Central");
        }
        else
        {
            // distribution from central
            // add an extra day to the time to allow for inbound new inventory -> central transfers...
            distributionWidget->setExecutionTime(time + 1);
            distributionWidget->setSource("Central");
            distributionWidget->setDestination(simulation->getNodeName(nodeId));
        }

        distributionWidget->setQuantity(releaseAmount);

        distributionWidget->execute();

        stockpileNetworkDistributionWidgets_.push_back(distributionWidget);
        layout_.insertWidget(2, distributionWidget);

        // wait for any GUI events to be processed
        QCoreApplication::processEvents();
    }
}

void StockpileNetworkWidget::addDistribution()
{
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        StockpileNetworkDistributionWidget * distributionWidget = new StockpileNetworkDistributionWidget(simulation);

        stockpileNetworkDistributionWidgets_.push_back(distributionWidget);
        layout_.insertWidget(2, distributionWidget);
    }
    else
    {
        put_flog(LOG_ERROR, "not a valid simulation");

        QMessageBox::warning(this, "Error", "Not a valid simulation.", QMessageBox::Ok, QMessageBox::Ok);
    }
}
