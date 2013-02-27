#ifndef STOCKPILE_NETWORK_H
#define STOCKPILE_NETWORK_H

#include "Stockpile.h"
#include <QtGui>
#include <vector>
#include <boost/shared_ptr.hpp>

class StockpileNetworkDistribution;

class StockpileNetwork : public QObject
{
    Q_OBJECT

    public:

        StockpileNetwork();

        void addStockpile(boost::shared_ptr<Stockpile> stockpile);
        void addDistribution(boost::shared_ptr<StockpileNetworkDistribution> distribution);

        std::vector<boost::shared_ptr<Stockpile> > getStockpiles();

        void evolve(int nowTime);

    signals:

        void changed();

    private:

        std::vector<boost::shared_ptr<Stockpile> > stockpiles_;
        std::vector<boost::shared_ptr<StockpileNetworkDistribution> > distributions_;
};

#endif
