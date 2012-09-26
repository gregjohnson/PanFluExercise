#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtGui>
#include <boost/shared_ptr.hpp>

class EpidemicDataSet;

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:

        MainWindow();

        QSize sizeHint() const;

    public slots:

        void openDataSet();

    private:

        boost::shared_ptr<EpidemicDataSet> dataSet_;
};

#endif
