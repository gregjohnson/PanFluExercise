#include "StockpileNetwork.h"
#include "StockpileNetworkDistribution.h"

StockpileNetwork::StockpileNetwork()
{

}

void StockpileNetwork::addStockpile(boost::shared_ptr<Stockpile> stockpile)
{
    connect(stockpile.get(), SIGNAL(changed()), this, SIGNAL(changed()));

    stockpiles_.push_back(stockpile);

    emit(changed());
}

void StockpileNetwork::addDistribution(boost::shared_ptr<StockpileNetworkDistribution> distribution)
{
    distributions_.push_back(distribution);
}

std::vector<boost::shared_ptr<Stockpile> > StockpileNetwork::getStockpiles()
{
    return stockpiles_;
}

void StockpileNetwork::evolve(int nowTime)
{
    // add new timestep to stockpiles
    for(unsigned int i=0; i<stockpiles_.size(); i++)
    {
        stockpiles_[i]->copyToNewTimeStep();
    }

    // apply distributions
    // for now, don't delete any of the distribution objects -- leave them in place
    for(unsigned int i=0; i<distributions_.size(); i++)
    {
        // apply() only does something at the appropriate nowTime
        distributions_[i]->apply(nowTime);
    }
}
