#include "StockpileNetworkDistributionWidget.h"
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
    executionTime_ = 0;

    setTitle("Distribution");

    QFormLayout * layout = new QFormLayout();
    setLayout(layout);

    // add widgets...

    // source stockpile

    // "new inventory" entry
    // use a NULL stockpile value
    QVariant newInventoryStockpilesVariant = QVariant::fromValue(boost::shared_ptr<Stockpile>());
    sourceComboBox_.addItem("New Inventory", newInventoryStockpilesVariant);

    std::vector<boost::shared_ptr<Stockpile> > stockpiles = dataSet->getStockpileNetwork()->getStockpiles();

    for(unsigned int i=0; i<stockpiles.size(); i++)
    {
        QVariant stockpileVariant = QVariant::fromValue(stockpiles[i]);
        sourceComboBox_.addItem(stockpiles[i]->getName().c_str(), stockpileVariant);
    }

    // also add node stockpiles
    std::vector<int> nodeIds = dataSet->getNodeIds();

    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        boost::shared_ptr<Stockpile> stockpile = dataSet->getStockpileNetwork()->getNodeStockpile(nodeIds[i]);

        QVariant stockpileVariant = QVariant::fromValue(stockpile);
        sourceComboBox_.addItem(stockpile->getName().c_str(), stockpileVariant);
    }

    layout->addRow("Source", &sourceComboBox_);

    // destination stockpile

    // "all" entry
    // use a NULL stockpile value
    QVariant allStockpilesVariant = QVariant::fromValue(boost::shared_ptr<Stockpile>());
    destinationComboBox_.addItem("All", allStockpilesVariant);

    for(unsigned int i=0; i<stockpiles.size(); i++)
    {
        QVariant stockpileVariant = QVariant::fromValue(stockpiles[i]);
        destinationComboBox_.addItem(stockpiles[i]->getName().c_str(), stockpileVariant);
    }

    // also add node stockpiles
    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        boost::shared_ptr<Stockpile> stockpile = dataSet->getStockpileNetwork()->getNodeStockpile(nodeIds[i]);

        QVariant stockpileVariant = QVariant::fromValue(stockpile);
        destinationComboBox_.addItem(stockpile->getName().c_str(), stockpileVariant);
    }

    layout->addRow("Destination", &destinationComboBox_);

    // type
    for(unsigned int i=0; i<NUM_STOCKPILE_TYPES; i++)
    {
        typeComboBox_.addItem(Stockpile::getTypeName((STOCKPILE_TYPE)i).c_str());
    }

    layout->addRow("Type", &typeComboBox_);

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

void StockpileNetworkDistributionWidget::setSource(std::string name)
{
    int index = sourceComboBox_.findText(name.c_str());

    if(index == -1)
    {
        put_flog(LOG_ERROR, "could not find %s", name.c_str());
    }
    else
    {
        sourceComboBox_.setCurrentIndex(index);
    }
}

void StockpileNetworkDistributionWidget::setDestination(std::string name)
{
    int index = destinationComboBox_.findText(name.c_str());

    if(index == -1)
    {
        put_flog(LOG_ERROR, "could not find %s", name.c_str());
    }
    else
    {
        destinationComboBox_.setCurrentIndex(index);
    }
}

void StockpileNetworkDistributionWidget::setQuantity(int quantity)
{
    quantitySpinBox_.setValue(quantity);
}

void StockpileNetworkDistributionWidget::setExecutionTime(int time)
{
    executionTime_ = time;
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

    // override execution time if set
    if(executionTime_ != 0)
    {
        time = executionTime_;
    }

    // source and destination stockpiles
    boost::shared_ptr<Stockpile> sourceStockpile = sourceComboBox_.itemData(sourceComboBox_.currentIndex()).value<boost::shared_ptr<Stockpile> >();
    boost::shared_ptr<Stockpile> destinationStockpile = destinationComboBox_.itemData(destinationComboBox_.currentIndex()).value<boost::shared_ptr<Stockpile> >();

    // create the distribution object
    boost::shared_ptr<StockpileNetworkDistribution> distribution(new StockpileNetworkDistribution(time, sourceStockpile, destinationStockpile, (STOCKPILE_TYPE)typeComboBox_.currentIndex(), quantitySpinBox_.value(), transferTimeSpinBox_.value()));

    // add the distribution to the network for later application
    dataSet_->getStockpileNetwork()->addDistribution(distribution);

    // update the result label
    QString label = "Distribution at time = " + QString::number(time);
    resultLabel_.setText(label);

    // connect signal so we get the clamped quantity when the transfer occurs
    connect(distribution.get(), SIGNAL(applied(int)), this, SLOT(applied(int)));
}
