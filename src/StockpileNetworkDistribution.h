#ifndef STOCKPILE_NETWORK_DISTRIBUTION_H
#define STOCKPILE_NETWORK_DISTRIBUTION_H

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <QtGui>

class Stockpile;
class StockpileNetwork;

class StockpileNetworkDistribution : public QObject
{
    Q_OBJECT

    public:

        StockpileNetworkDistribution(int time, boost::shared_ptr<Stockpile> sourceStockpile, boost::shared_ptr<Stockpile> destinationStockpile, int quantity, int transferTime);

        void setNetwork(boost::shared_ptr<StockpileNetwork> network);

        // execute the distribution if nowTime == time_, time_ + transferTime_
        void apply(int nowTime);

        int getTime();
        boost::shared_ptr<Stockpile> getSourceStockpile();
        int getQuantity();
        int getTransferTime();

        // the total quantity from the source
        int getClampedQuantity();

        // we can have multiple destination stockpiles
        // to avoid confusion, this is why we don't have a getDestinationStockpile()...
        bool hasDestinationStockpile(boost::shared_ptr<Stockpile> stockpile);
        int getClampedQuantity(boost::shared_ptr<Stockpile> destinationStockpile);

    signals:

        void applied(int clampedQuanity);

    private:

        // the network this distribution is associated with; weak_ptr to prevent cyclic references
        boost::weak_ptr<StockpileNetwork> network_;

        // time at which distribution was executed ("now")
        int time_;

        boost::shared_ptr<Stockpile> sourceStockpile_; // NULL == new inventory
        boost::shared_ptr<Stockpile> destinationStockpile_; // NULL == to all stockpiles associated with a set of nodeIds
        int quantity_;
        int transferTime_;

        // the actual quantity transferred, clamped based on actual inventory at the time of transfer from source
        int clampedQuantity_;

        // if destination stockpile is NULL, that indicates we're transferring to all stockpiles associated with a set of nodeIds
        std::map<boost::shared_ptr<Stockpile>, int> clampedQuantities_;
};

#endif
