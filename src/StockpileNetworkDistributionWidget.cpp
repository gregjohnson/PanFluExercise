#include "StockpileNetworkDistributionWidget.h"
#include "StockpileWidget.h"
#include "EpidemicDataSet.h"
#include "EpidemicSimulation.h"
#include "StockpileNetwork.h"
#include "StockpileNetworkDistribution.h"
#include "log.h"

// need Stockpile shared_ptr allowed as a QVariant
Q_DECLARE_METATYPE(boost::shared_ptr<Stockpile>)

StockpileNetworkDistributionWidget::StockpileNetworkDistributionWidget(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    setTitle("Distribution");

    QFormLayout * layout = new QFormLayout();
    setLayout(layout);

    // add widgets...

    // source stockpile
    std::vector<boost::shared_ptr<Stockpile> > stockpiles = dataSet->getStockpileNetwork()->getStockpiles();

    for(unsigned int i=0; i<stockpiles.size(); i++)
    {
        QVariant stockpileVariant = QVariant::fromValue(stockpiles[i]);
        sourceComboBox_.addItem(stockpiles[i]->getName().c_str(), stockpileVariant);
    }

    layout->addRow("Source", &sourceComboBox_);

    // destination stockpile
    for(unsigned int i=0; i<stockpiles.size(); i++)
    {
        QVariant stockpileVariant = QVariant::fromValue(stockpiles[i]);
        destinationComboBox_.addItem(stockpiles[i]->getName().c_str(), stockpileVariant);
    }

    layout->addRow("Destination", &destinationComboBox_);

    // quantity
    quantitySpinBox_.setMaximum(STOCKPILE_WIDGET_NUM_MAX);

    layout->addRow("Quantity", &quantitySpinBox_);

    // transfer time
    transferTimeSpinBox_.setMaximum(365);
    transferTimeSpinBox_.setSuffix(" days");

    layout->addRow("Transfer Time", &transferTimeSpinBox_);

    // results label
    layout->addWidget(&resultLabel_);

    // execute button
    QPushButton * executeButton = new QPushButton("Execute", this);
    layout->addWidget(executeButton);

    // connections
    connect(executeButton, SIGNAL(clicked()), this, SLOT(execute()));
}

StockpileNetworkDistributionWidget::~StockpileNetworkDistributionWidget()
{

}

void StockpileNetworkDistributionWidget::applied(int clampedQuantity)
{
    resultLabel_.setText(resultLabel_.text() + "\nActual quantity: " + QString::number(clampedQuantity));
}

void StockpileNetworkDistributionWidget::execute()
{
    // disable the widgets for further modification
    setEnabled(false);

    // bounds on transfer quantities taken when transfer is applied; no need to consider here

    // the "now" time
    int time = dataSet_->getNumTimes();

    // source and destination stockpiles
    boost::shared_ptr<Stockpile> sourceStockpile = sourceComboBox_.itemData(sourceComboBox_.currentIndex()).value<boost::shared_ptr<Stockpile> >();
    boost::shared_ptr<Stockpile> destinationStockpile = destinationComboBox_.itemData(destinationComboBox_.currentIndex()).value<boost::shared_ptr<Stockpile> >();

    // create the distribution object
    boost::shared_ptr<StockpileNetworkDistribution> distribution(new StockpileNetworkDistribution(time, sourceStockpile, destinationStockpile, quantitySpinBox_.value(), transferTimeSpinBox_.value()));

    // add the distribution to the network for later application
    dataSet_->getStockpileNetwork()->addDistribution(distribution);

    // update the result label
    QString label = "Executed at time = " + QString::number(time);
    resultLabel_.setText(label);

    // connect signal so we get the clamped quantity when the transfer occurs
    connect(distribution.get(), SIGNAL(applied(int)), this, SLOT(applied(int)));
}
