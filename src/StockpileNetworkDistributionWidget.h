#ifndef STOCKPILE_NETWORK_DISTRIBUTION_WIDGET_H
#define STOCKPILE_NETWORK_DISTRIBUTION_WIDGET_H

#define STOCKPILE_WIDGET_NUM_MAX 99999999

#include <QtGui>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;

class StockpileNetworkDistributionWidget : public QGroupBox
{
    Q_OBJECT

    public:

        StockpileNetworkDistributionWidget(boost::shared_ptr<EpidemicDataSet> dataSet);
        ~StockpileNetworkDistributionWidget();

        void setSource(std::string name);
        void setDestination(std::string name);
        void setQuantity(int quantity);
        void setExecutionTime(int time);

    public slots:

        void applied(int clampedQuantity);
        void execute();

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;

        // override the "now" execution time
        int executionTime_;

        // UI elements
        QComboBox sourceComboBox_;
        QComboBox destinationComboBox_;
        QComboBox typeComboBox_;
        QSpinBox quantitySpinBox_;
        QSpinBox transferTimeSpinBox_;
        QLabel resultLabel_;
};

#endif
