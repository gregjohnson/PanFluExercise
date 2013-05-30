#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

// delay between moving to next timestep when playing
#define PLAY_TIMESTEPS_TIMER_DELAY_MILLISECONDS 100

#include <QtGui>
#include <boost/shared_ptr.hpp>

class EpidemicDataSet;
class EpidemicInitialCasesWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:

        MainWindow();
        ~MainWindow();

        QSize sizeHint() const;

    signals:

        void dataSetChanged(boost::shared_ptr<EpidemicDataSet> dataSet=boost::shared_ptr<EpidemicDataSet>());
        void numberOfTimestepsChanged();
        void timeChanged(int time);

    public slots:

        void setTime(int time);
        bool previousTimestep();
        void playTimesteps(bool set=true);
        bool nextTimestep();

    private:

        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;

        QSlider * timeSlider_;

        QAction * playTimestepsAction_;
        QTimer playTimestepsTimer_;

        EpidemicInitialCasesWidget * initialCasesWidget_;

    private slots:

        void newSimulation();
        void openDataSet();
        void newChart();
        void resetTimeSlider();
        void connectToDisplayCluster();
        void disconnectFromDisplayCluster();
};

#endif
