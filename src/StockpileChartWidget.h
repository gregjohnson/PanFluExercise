#ifndef STOCKPILE_CHART_WIDGET_H
#define STOCKPILE_CHART_WIDGET_H

#define STOCKPILE_CHART_MODE_CURRENT 0
#define STOCKPILE_CHART_MODE_TIME_HISTORY 1

#include "ChartWidget.h"
#include "Stockpile.h"
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
        void setTime(int time);

        void update();

    private:

        void updateBarChart();
        void updateTimeSeries();

        // the chart widget
        ChartWidget chartWidget_;

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;
        boost::shared_ptr<StockpileNetwork> stockpileNetwork_;

        // chart mode
        int mode_;

        // selected stockpile type
        STOCKPILE_TYPE type_;

        // time indicator line
        boost::shared_ptr<ChartWidgetLine> timeIndicator_;

    private slots:

        void setModeChoice(int choiceIndex);
        void setTypeChoice(int choiceIndex);
};

#endif
