#include "StockpileChartWidget.h"
#include "EpidemicDataSet.h"
#include "StockpileNetwork.h"
#include "StockpileNetworkDistribution.h"
#include "Stockpile.h"
#include "log.h"
#include <string>
#include <vector>

StockpileChartWidget::StockpileChartWidget(MainWindow * mainWindow)
{
    // defaults
    time_ = 0;
    mode_ = STOCKPILE_CHART_MODE_CURRENT;

    // add toolbar
    QToolBar * toolbar = addToolBar("toolbar");

    // mode combobox
    QComboBox * modeComboBox = new QComboBox(this);

    // should correspond to #defines, STOCKPILE_CHART_MODE_CURRENT...
    modeComboBox->addItem("Current Inventory");
    modeComboBox->addItem("Time History");

    connect(modeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setModeChoice(int)));

    // add mode combobox to toolbar
    toolbar->addWidget(modeComboBox);

    setCentralWidget(&chartWidget_);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect((QObject *)mainWindow, SIGNAL(numberOfTimestepsChanged()), this, SLOT(update()));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));
}

void StockpileChartWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    if(dataSet != NULL)
    {
        stockpileNetwork_ = dataSet->getStockpileNetwork();
    }
    else
    {
        // NULL
        stockpileNetwork_ = boost::shared_ptr<StockpileNetwork>();
    }

    update();
}

void StockpileChartWidget::setTime(int time)
{
    time_ = time;

    if(mode_ == STOCKPILE_CHART_MODE_CURRENT)
    {
        // update needed to grab current time step
        update();
    }
    else if(mode_ == STOCKPILE_CHART_MODE_TIME_HISTORY && dataSet_ != NULL && timeIndicator_ != NULL)
    {
        // don't do a full update, just update the time indicator line
        timeIndicator_->clear();

        timeIndicator_->addPoint(time_, 0);
        timeIndicator_->addPoint(time_, 999999999.);
    }

    chartWidget_.exportSVGToDisplayCluster();
}

void StockpileChartWidget::update()
{
    if(mode_ == STOCKPILE_CHART_MODE_CURRENT)
    {
        // no time indicator line needed
        if(timeIndicator_ != NULL)
        {
            timeIndicator_->clear();
        }

        updateBarChart();
    }
    else if(mode_ == STOCKPILE_CHART_MODE_TIME_HISTORY)
    {
        updateTimeSeries();
    }
    else
    {
        put_flog(LOG_ERROR, "unknown mode %i", mode_);
        return;
    }
}

void StockpileChartWidget::updateBarChart()
{
    // clear current plots
    chartWidget_.clear();

    // set x-axis label
    std::string xAxisLabel("Location");
    chartWidget_.setXAxisLabel(xAxisLabel);

    // set y-axis label
    std::string yAxisLabel("Stockpile");
    chartWidget_.setYAxisLabel(yAxisLabel);

    if(stockpileNetwork_ != NULL)
    {
        chartWidget_.setTitle("All Stockpiles");

        std::vector<boost::shared_ptr<Stockpile> > stockpiles = stockpileNetwork_->getStockpiles();

        std::vector<boost::shared_ptr<StockpileNetworkDistribution> > pendingDistributions = stockpileNetwork_->getPendingDistributions(time_);

        // plot the variable
        boost::shared_ptr<ChartWidgetLine> line = chartWidget_.getLine(NEW_LINE, BAR);

        // series labels
        std::vector<std::string> seriesLabels;
        seriesLabels.push_back("Inventory");
        seriesLabels.push_back("Outbound");
        seriesLabels.push_back("Inbound");
        seriesLabels.push_back("Usable");

        line->setLabels(seriesLabels);

        // labels for each bar
        std::vector<std::string> labels;

        for(unsigned int i=0; i<stockpiles.size(); i++)
        {
            int currentQuantity = stockpiles[i]->getNum(time_);
            int pendingOutQuantity = 0;
            int pendingInQuantity = 0;

            for(unsigned int j=0; j<pendingDistributions.size(); j++)
            {
                if(pendingDistributions[j]->getSourceStockpile() == stockpiles[i])
                {
                    pendingOutQuantity += pendingDistributions[j]->getClampedQuantity();
                }

                if(pendingDistributions[j]->hasDestinationStockpile(stockpiles[i]) == true)
                {
                    pendingInQuantity += pendingDistributions[j]->getClampedQuantity(stockpiles[i]);
                }
            }

            int usable = 0;

            std::vector<int> nodeIds = stockpiles[i]->getNodeIds();

            for(unsigned int j=0; j<nodeIds.size(); j++)
            {
                usable += stockpileNetwork_->getNodeStockpile(nodeIds[j])->getNum(time_);
            }

            std::vector<double> points;

            points.push_back(currentQuantity);
            points.push_back(pendingOutQuantity);
            points.push_back(pendingInQuantity);
            points.push_back(usable);

            labels.push_back(stockpiles[i]->getName());
            line->addPoints(i, points);
        }

        line->setBarLabels(labels);

        // reset chart bounds
        chartWidget_.resetBounds();
    }
}

void StockpileChartWidget::updateTimeSeries()
{
    // clear current plots
    chartWidget_.clear();

    // set x-axis label
    std::string xAxisLabel("Time (days)");
    chartWidget_.setXAxisLabel(xAxisLabel);

    // set y-axis label
    std::string yAxisLabel("Stockpile");
    chartWidget_.setYAxisLabel(yAxisLabel);

    if(dataSet_ != NULL && stockpileNetwork_ != NULL)
    {
        chartWidget_.setTitle("All Stockpiles");

        // add a (0,0) point to fix bounds calculations for straight horizontal plots
        boost::shared_ptr<ChartWidgetLine> line0 = chartWidget_.getLine();
        line0->setLabel("");
        line0->addPoint(0, 0);

        // add by group
        std::vector<boost::shared_ptr<Stockpile> > stockpiles = stockpileNetwork_->getStockpiles();

        // plot the variable
        boost::shared_ptr<ChartWidgetLine> line = chartWidget_.getLine(NEW_LINE, STACKED);

        line->setWidth(2.);

        // clear any existing bar labels
        line->clearBarLabels();

        std::vector<std::string> labels;

        for(unsigned int i=0; i<stockpiles.size(); i++)
        {
            labels.push_back("Stockpile (" + stockpiles[i]->getName() + ")");
        }

        line->setLabels(labels);

        for(int t=0; t<dataSet_->getNumTimes(); t++)
        {
            std::vector<double> variableValues;

            for(unsigned int i=0; i<stockpiles.size(); i++)
            {
                // inventory for this stockpile
                int stockpile = stockpiles[i]->getNum(t);

                // usable from node stockpiles
                int usable = 0;

                std::vector<int> nodeIds = stockpiles[i]->getNodeIds();

                for(unsigned int j=0; j<nodeIds.size(); j++)
                {
                    usable += stockpileNetwork_->getNodeStockpile(nodeIds[j])->getNum(t);
                }

                // include inventory for this stockpile and from the node stockpiles
                variableValues.push_back((double)stockpile + (double)usable);
            }

            line->addPoints(t, variableValues);
        }

        // clear time indicator
        timeIndicator_ = chartWidget_.getLine();
        timeIndicator_->setWidth(2.);
        timeIndicator_->setLabel("");

        // reset chart bounds
        chartWidget_.resetBounds();
    }
}

void StockpileChartWidget::setModeChoice(int choiceIndex)
{
    mode_ = choiceIndex;

    update();
}
