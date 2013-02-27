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

    private:

        // time at which distribution was executed ("now")
        int time_;

        boost::shared_ptr<Stockpile> sourceStockpile_;
        boost::shared_ptr<Stockpile> destinationStockpile_;
        int quantity_;
        int transferTime_;
};

#endif
