#include "StockpileNetwork.h"
#include "EpidemicDataSet.h"
#include "StockpileNetworkDistribution.h"
#include "log.h"
#include <boost/lexical_cast.hpp>

StockpileNetwork::StockpileNetwork(EpidemicDataSet * dataSet)
{
    dataSet_ = dataSet;

    std::vector<int> nodeIds = dataSet->getNodeIds();

    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        boost::shared_ptr<Stockpile> stockpile(new Stockpile(boost::lexical_cast<std::string>(nodeIds[i])));

        nodeStockpiles_[nodeIds[i]] = stockpile;
    }
}

void StockpileNetwork::addStockpile(boost::shared_ptr<Stockpile> stockpile)
{
    stockpiles_.push_back(stockpile);
}

void StockpileNetwork::addDistribution(boost::shared_ptr<StockpileNetworkDistribution> distribution)
{
    // associate this network with the distribution
    distribution->setNetwork(shared_from_this());

    distributions_.push_back(distribution);
}

EpidemicDataSet * StockpileNetwork::getDataSet()
{
    return dataSet_;
}

std::vector<boost::shared_ptr<Stockpile> > StockpileNetwork::getStockpiles()
{
    return stockpiles_;
}

std::vector<boost::shared_ptr<StockpileNetworkDistribution> > StockpileNetwork::getPendingDistributions(int nowTime)
{
    std::vector<boost::shared_ptr<StockpileNetworkDistribution> > pendingDistributions;

    for(unsigned int i=0; i<distributions_.size(); i++)
    {
        if(nowTime >= distributions_[i]->getTime() && nowTime < distributions_[i]->getTime() + distributions_[i]->getTransferTime())
        {
            pendingDistributions.push_back(distributions_[i]);
        }
    }

    return pendingDistributions;
}

boost::shared_ptr<Stockpile> StockpileNetwork::getNodeStockpile(int nodeId)
{
    if(nodeStockpiles_.count(nodeId) == 0)
    {
        put_flog(LOG_ERROR, "no node stockpile for nodeId %i", nodeId);

        return boost::shared_ptr<Stockpile>();
    }

    return nodeStockpiles_[nodeId];
}

void StockpileNetwork::evolve(int nowTime)
{
    // add new timestep to stockpiles
    for(unsigned int i=0; i<stockpiles_.size(); i++)
    {
        stockpiles_[i]->copyToNewTimeStep();
    }

    std::map<int, boost::shared_ptr<Stockpile> >::iterator iter;

    for(iter=nodeStockpiles_.begin(); iter!=nodeStockpiles_.end(); iter++)
    {
        iter->second->copyToNewTimeStep();
    }

    // apply distributions
    // for now, don't delete any of the distribution objects -- leave them in place
    for(unsigned int i=0; i<distributions_.size(); i++)
    {
        // apply() only does something at the appropriate nowTime
        distributions_[i]->apply(nowTime);
    }
}
