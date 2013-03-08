#ifndef STOCKPILE_NETWORK_DISTRIBUTION_WIDGET_H
#define STOCKPILE_NETWORK_DISTRIBUTION_WIDGET_H

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

    public slots:

        void applied(int clampedQuantity);

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;

        // UI elements
        QComboBox sourceComboBox_;
        QComboBox destinationComboBox_;
        QSpinBox quantitySpinBox_;
        QSpinBox transferTimeSpinBox_;
        QLabel resultLabel_;

    private slots:

        void execute();
};

#endif
