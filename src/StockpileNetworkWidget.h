#ifndef STOCKPILE_NETWORK_WIDGET_H
#define STOCKPILE_NETWORK_WIDGET_H

#include <QtGui>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;
class StockpileWidget;

class StockpileNetworkWidget : public QScrollArea
{
    Q_OBJECT

    public:

        StockpileNetworkWidget(MainWindow * mainWindow);
        ~StockpileNetworkWidget();

    public slots:

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;

        // UI elements
        QVBoxLayout layout_;
        std::vector<StockpileWidget *> stockpileWidgets_;
};

#endif
