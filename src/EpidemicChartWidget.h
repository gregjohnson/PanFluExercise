#ifndef EPIDEMIC_CHART_WIDGET_H
#define EPIDEMIC_CHART_WIDGET_H

#include "ChartWidget.h"
#include <QtGui>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;

class EpidemicChartWidget : public ChartWidget
{
    Q_OBJECT

    public:

        EpidemicChartWidget(MainWindow * mainWindow);

    public slots:

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        void setTime(int time);
        void setNodeId(int nodeId);

        void update();

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;
        int nodeId_;

        // time indicator line
        boost::shared_ptr<ChartWidgetLine> timeIndicator_;
};

#endif
