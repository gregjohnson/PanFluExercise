#ifndef STOCKPILE_NETWORK_H
#define STOCKPILE_NETWORK_H

#include "Stockpile.h"
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class EpidemicDataSet;
class StockpileNetworkDistribution;

class StockpileNetwork : public boost::enable_shared_from_this<StockpileNetwork>
{
    public:

        StockpileNetwork(EpidemicDataSet * dataSet);

        void addStockpile(boost::shared_ptr<Stockpile> stockpile);
        void addDistribution(boost::shared_ptr<StockpileNetworkDistribution> distribution);

        EpidemicDataSet * getDataSet();

        std::vector<boost::shared_ptr<Stockpile> > getStockpiles();

        std::vector<boost::shared_ptr<StockpileNetworkDistribution> > getPendingDistributions(int nowTime);

        boost::shared_ptr<Stockpile> getNodeStockpile(int nodeId);

        void evolve(int nowTime);

    private:

        // only a raw pointer since the data set owns this object
        EpidemicDataSet * dataSet_;

        std::vector<boost::shared_ptr<Stockpile> > stockpiles_;
        std::vector<boost::shared_ptr<StockpileNetworkDistribution> > distributions_;

        // local stockpiles for each node
        // these stockpiles are made available for interventions
        std::map<int, boost::shared_ptr<Stockpile> > nodeStockpiles_;
};

#endif
