#include "StockpileChartWidget.h"
#include "EpidemicDataSet.h"
#include "StockpileNetwork.h"
#include "Stockpile.h"
#include <string>
#include <vector>

StockpileChartWidget::StockpileChartWidget(MainWindow * mainWindow)
{
    // defaults
    time_ = 0;

    setCentralWidget(&chartWidget_);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));
}

void StockpileChartWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    if(dataSet != NULL)
    {
        stockpileNetwork_ = dataSet->getStockpileNetwork();

        // todo: disconnect existing signals/slots

        if(stockpileNetwork_ != NULL)
        {
            connect(stockpileNetwork_.get(), SIGNAL(changed()), this, SLOT(update()));
        }
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

    update();

    chartWidget_.exportSVGToDisplayCluster();
}

void StockpileChartWidget::update()
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

        // plot the variable
        boost::shared_ptr<ChartWidgetLine> line = chartWidget_.getLine(NEW_LINE, BAR);

        line->setColor(1.,0.,0.);
        line->setWidth(2.);

        // labels for each bar
        std::vector<std::string> labels;

        for(unsigned int i=0; i<stockpiles.size(); i++)
        {
            labels.push_back(stockpiles[i]->getName());
            line->addPoint(i, stockpiles[i]->getNum(time_));
        }

        line->setBarLabels(labels);

        // reset chart bounds
        chartWidget_.resetBounds();
    }
}
