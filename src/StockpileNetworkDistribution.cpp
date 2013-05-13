#include "StockpileNetworkDistribution.h"
#include "Stockpile.h"
#include "StockpileNetwork.h"
#include "EpidemicDataSet.h"
#include "log.h"

StockpileNetworkDistribution::StockpileNetworkDistribution(int time, boost::shared_ptr<Stockpile> sourceStockpile, boost::shared_ptr<Stockpile> destinationStockpile, int quantity, int transferTime)
{
    // defaults
    clampedQuantity_ = -1;

    time_ = time;
    sourceStockpile_ = sourceStockpile;
    destinationStockpile_ = destinationStockpile;
    quantity_ = quantity;
    transferTime_ = transferTime;
}

void StockpileNetworkDistribution::setNetwork(boost::shared_ptr<StockpileNetwork> network)
{
    network_ = network;
}

void StockpileNetworkDistribution::apply(int nowTime)
{
    if(nowTime == time_)
    {
        int clampedQuantity = quantity_;

        if(sourceStockpile_ != NULL)
        {
            if(clampedQuantity > sourceStockpile_->getNum(nowTime))
            {
                put_flog(LOG_INFO, "clamping transfer quantity to %i", sourceStockpile_->getNum(nowTime));

                clampedQuantity = sourceStockpile_->getNum(nowTime);
            }

            // decrement source
            sourceStockpile_->setNum(nowTime, sourceStockpile_->getNum(nowTime) - clampedQuantity);
        }

        // save clamped quantity
        clampedQuantity_ = clampedQuantity;

        // emit signal
        emit(applied(clampedQuantity_));

        // name of source
        std::string sourceName;

        if(sourceStockpile_ != NULL)
        {
            sourceName = sourceStockpile_->getName();
        }
        else
        {
            sourceName = "New Inventory";
        }

        if(destinationStockpile_ != NULL)
        {
            put_flog(LOG_INFO, "applying distribution (outbound): %s --> %s, %i", sourceName.c_str(), destinationStockpile_->getName().c_str(), clampedQuantity_);

            // go ahead and save to the map too, to simplify the inbound distribution
            clampedQuantities_[destinationStockpile_] = clampedQuantity_;
        }
        else
        {
            // distribution to all stockpiles associated with a set of nodeIds
            // set clamped quantities for each destination stockpile

            // make sure we're associated with a network
            boost::shared_ptr<StockpileNetwork> network = network_.lock();

            if(network == NULL)
            {
                put_flog(LOG_ERROR, "not associated with a network");
                return;
            }

            // total population
            float totalPopulation = network->getDataSet()->getPopulation(network->getDataSet()->getNodeIds());

            std::vector<boost::shared_ptr<Stockpile> > stockpiles = network->getStockpiles();

            for(unsigned int i=0; i<stockpiles.size(); i++)
            {
                if(stockpiles[i]->getNodeIds().size() > 0)
                {
                    // population for nodeIds of this stockpile
                    float stockpilePopulation = network->getDataSet()->getPopulation(stockpiles[i]->getNodeIds());

                    // prorata to this stockpile by population
                    clampedQuantities_[stockpiles[i]] = (int)(stockpilePopulation / totalPopulation * (float)clampedQuantity_);

                    put_flog(LOG_INFO, "applying split distribution (outbound): %s --> %s, %i", sourceName.c_str(), stockpiles[i]->getName().c_str(), clampedQuantities_[stockpiles[i]]);
                }
            }
        }
    }

    if(nowTime == time_ + transferTime_)
    {
        // name of source
        std::string sourceName;

        if(sourceStockpile_ != NULL)
        {
            sourceName = sourceStockpile_->getName();
        }
        else
        {
            sourceName = "New Inventory";
        }

        for(std::map<boost::shared_ptr<Stockpile>, int>::iterator it=clampedQuantities_.begin(); it!=clampedQuantities_.end(); it++)
        {
            boost::shared_ptr<Stockpile> destinationStockpile = it->first;
            int clampedQuantity = it->second;

            put_flog(LOG_INFO, "applying distribution (inbound): %s --> %s, %i", sourceName.c_str(), destinationStockpile->getName().c_str(), clampedQuantity);

            // increment destination
            destinationStockpile->setNum(nowTime, destinationStockpile->getNum(nowTime) + clampedQuantity);

            // if the destination corresponds to a group of nodes, distribute to nodes
            std::vector<int> destinationNodeIds = destinationStockpile->getNodeIds();

            if(destinationNodeIds.size() > 0)
            {
                // make sure we're associated with a network
                boost::shared_ptr<StockpileNetwork> network = network_.lock();

                if(network == NULL)
                {
                    put_flog(LOG_ERROR, "not associated with a network");
                    return;
                }

                // at the end of the distribution the destination stockpile should be this
                int destinationStockpileFinal = destinationStockpile->getNum(nowTime) - clampedQuantity;

                // total population
                float totalPopulation = network->getDataSet()->getPopulation(destinationNodeIds);

                for(unsigned int i=0; i<destinationNodeIds.size(); i++)
                {
                    // population and stockpile for this nodeId
                    float population = network->getDataSet()->getPopulation(destinationNodeIds[i]);

                    boost::shared_ptr<Stockpile> nodeStockpile = network->getNodeStockpile(destinationNodeIds[i]);

                    if(nodeStockpile == NULL)
                    {
                        put_flog(LOG_ERROR, "NULL node stockpile for nodeId %i", destinationNodeIds[i]);
                        continue;
                    }

                    // now, make the pro-rata distribution
                    float fraction = population / totalPopulation;

                    // todo: this truncates the decimal quantity...
                    int clampedQuantityFraction = (int)(fraction * (float)clampedQuantity);

                    put_flog(LOG_INFO, "applying distribution (pro rata to nodes): %s --> %s, %i", destinationStockpile->getName().c_str(), nodeStockpile->getName().c_str(), clampedQuantityFraction);

                    // decrement original destination
                    destinationStockpile->setNum(nowTime, destinationStockpile->getNum(nowTime) - clampedQuantityFraction);

                    // increment node stockpile
                    nodeStockpile->setNum(nowTime, nodeStockpile->getNum(nowTime) + clampedQuantityFraction);
                }

                // make sure the destination stockpile has the expected quantity
                // if not, correct it... this occurs due to integer division issues
                if(destinationStockpile->getNum(nowTime) != destinationStockpileFinal)
                {
                    put_flog(LOG_DEBUG, "adjust desination stockpile to %i from %i", destinationStockpileFinal, destinationStockpile->getNum(nowTime));

                    destinationStockpile->setNum(nowTime, destinationStockpileFinal);
                }
            }
        }
    }
}

int StockpileNetworkDistribution::getTime()
{
    return time_;
}

boost::shared_ptr<Stockpile> StockpileNetworkDistribution::getSourceStockpile()
{
    return sourceStockpile_;
}

int StockpileNetworkDistribution::getQuantity()
{
    return quantity_;
}

int StockpileNetworkDistribution::getTransferTime()
{
    return transferTime_;
}

int StockpileNetworkDistribution::getClampedQuantity()
{
    if(clampedQuantity_ == -1)
    {
        return quantity_;
    }
    else
    {
        return clampedQuantity_;
    }
}

bool StockpileNetworkDistribution::hasDestinationStockpile(boost::shared_ptr<Stockpile> stockpile)
{
    if(clampedQuantities_.count(stockpile) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int StockpileNetworkDistribution::getClampedQuantity(boost::shared_ptr<Stockpile> destinationStockpile)
{
    if(clampedQuantities_.count(destinationStockpile) > 0)
    {
        return clampedQuantities_[destinationStockpile];
    }
    else
    {
        return 0;
    }
}
