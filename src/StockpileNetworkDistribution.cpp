#include "StockpileNetworkDistribution.h"
#include "Stockpile.h"
#include "log.h"

StockpileNetworkDistribution::StockpileNetworkDistribution(int time, boost::shared_ptr<Stockpile> sourceStockpile, boost::shared_ptr<Stockpile> destinationStockpile, int quantity, int transferTime)
{
    time_ = time;
    sourceStockpile_ = sourceStockpile;
    destinationStockpile_ = destinationStockpile;
    quantity_ = quantity;
    transferTime_ = transferTime;
}

void StockpileNetworkDistribution::apply(int nowTime)
{
    if(nowTime == time_ + transferTime_)
    {
        int clampedQuantity = quantity_;

        if(clampedQuantity > sourceStockpile_->getNum(nowTime))
        {
            put_flog(LOG_INFO, "clamping transfer quantity to %i", sourceStockpile_->getNum(nowTime));

            clampedQuantity = sourceStockpile_->getNum(nowTime);
        }

        put_flog(LOG_INFO, "applying distribution: %s --> %s, %i", sourceStockpile_->getName().c_str(), destinationStockpile_->getName().c_str(), clampedQuantity);

        // decrement source
        sourceStockpile_->setNum(nowTime, sourceStockpile_->getNum(nowTime) - clampedQuantity);

        // increment destination
        destinationStockpile_->setNum(nowTime, destinationStockpile_->getNum(nowTime) + clampedQuantity);
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

    }
}
