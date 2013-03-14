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

        if(clampedQuantity > sourceStockpile_->getNum(nowTime))
        {
            put_flog(LOG_INFO, "clamping transfer quantity to %i", sourceStockpile_->getNum(nowTime));

            clampedQuantity = sourceStockpile_->getNum(nowTime);
        }

        put_flog(LOG_INFO, "applying distribution (outbound): %s --> %s, %i", sourceStockpile_->getName().c_str(), destinationStockpile_->getName().c_str(), clampedQuantity);

        // decrement source
        sourceStockpile_->setNum(nowTime, sourceStockpile_->getNum(nowTime) - clampedQuantity);

        // save clamped quantity
        clampedQuantity_ = clampedQuantity;

        // emit signal
        emit(applied(clampedQuantity_));
    }

    if(nowTime == time_ + transferTime_)
    {
        put_flog(LOG_INFO, "applying distribution (inbound): %s --> %s, %i", sourceStockpile_->getName().c_str(), destinationStockpile_->getName().c_str(), clampedQuantity_);

        // increment destination
        destinationStockpile_->setNum(nowTime, destinationStockpile_->getNum(nowTime) + clampedQuantity_);

        // if the destination corresponds to a group of nodes, distribute to nodes
        std::vector<int> destinationNodeIds = destinationStockpile_->getNodeIds();

        if(destinationNodeIds.size() > 0)
        {
            // make sure we're associated with a network
            boost::shared_ptr<StockpileNetwork> network = network_.lock();

            if(network == NULL)
            {
                put_flog(LOG_ERROR, "not associated with a network");
                return;
            }

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
                int clampedQuantityFraction = (int)(fraction * clampedQuantity_);

                put_flog(LOG_INFO, "applying distribution (pro rata to nodes): %s --> %s, %i", destinationStockpile_->getName().c_str(), nodeStockpile->getName().c_str(), clampedQuantityFraction);

                // decrement original destination
                destinationStockpile_->setNum(nowTime, destinationStockpile_->getNum(nowTime) - clampedQuantityFraction);

                // increment node stockpile
                nodeStockpile->setNum(nowTime, nodeStockpile->getNum(nowTime) + clampedQuantityFraction);
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

boost::shared_ptr<Stockpile> StockpileNetworkDistribution::getDestinationStockpile()
{
    return destinationStockpile_;
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
