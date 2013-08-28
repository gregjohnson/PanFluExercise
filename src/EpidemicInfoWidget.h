#ifndef EPIDEMIC_INFO_WIDGET_H
#define EPIDEMIC_INFO_WIDGET_H

#include <QtGui>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;

class EpidemicInfoWidget : public QWidget {
    Q_OBJECT

    public:

        EpidemicInfoWidget(MainWindow * mainWindow);

    public slots:

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        void setTime(int time);

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;

        // UI elements
        QTableView tableView_;
        QStandardItemModel model_;

        void updateModel();
};

#endif
