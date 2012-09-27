#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ChartWidget.h"
#include <QtGui>
#include <boost/shared_ptr.hpp>

class EpidemicDataSet;
class MapWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:

        MainWindow();
        ~MainWindow();

        QSize sizeHint() const;

    signals:

        void dataSetChanged(boost::shared_ptr<EpidemicDataSet> dataSet=boost::shared_ptr<EpidemicDataSet>());
        void timeChanged(int time);

    public slots:

        void setTime(int time);

    private:

        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;

        MapWidget * mapWidget_;
        QSlider * timeSlider_;
        ChartWidget chartWidget_;

    private slots:

        void openDataSet();
        void resetTimeSlider();
        void updateChartWidget();
};

#endif
