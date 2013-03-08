#ifndef STOCKPILE_NETWORK_DISTRIBUTION_H
#define STOCKPILE_NETWORK_DISTRIBUTION_H

#include <boost/shared_ptr.hpp>

class Stockpile;

class StockpileNetworkDistribution
{
    public:

        StockpileNetworkDistribution(int time, boost::shared_ptr<Stockpile> sourceStockpile, boost::shared_ptr<Stockpile> destinationStockpile, int quantity, int transferTime);

        // execute the distribution if nowTime == time_ + transferTime_
        void apply(int nowTime);

        int getTime();
        boost::shared_ptr<Stockpile> getSourceStockpile();
        boost::shared_ptr<Stockpile> getDestinationStockpile();
        int getQuantity();
        int getTransferTime();

        int getClampedQuantity();

    private:

        // time at which distribution was executed ("now")
        int time_;

        boost::shared_ptr<Stockpile> sourceStockpile_;
        boost::shared_ptr<Stockpile> destinationStockpile_;
        int quantity_;
        int transferTime_;

        // the actual quantity transferred, clamped based on actual inventory at the time of transfer from source
        int clampedQuantity_;
};

#endif
