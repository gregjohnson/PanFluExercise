#ifndef STOCKPILE_CHART_WIDGET_H
#define STOCKPILE_CHART_WIDGET_H

#include "ChartWidget.h"
#include <QtGui>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;
class StockpileNetwork;

class StockpileChartWidget : public QMainWindow
{
    Q_OBJECT

    public:

        StockpileChartWidget(MainWindow * mainWindow);

    public slots:

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);

        void update();

    private:

        // the chart widget
        ChartWidget chartWidget_;

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;
        boost::shared_ptr<StockpileNetwork> stockpileNetwork_;
};

#endif
