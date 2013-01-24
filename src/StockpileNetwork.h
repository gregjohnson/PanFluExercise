#ifndef STOCKPILE_NETWORK_H
#define STOCKPILE_NETWORK_H

#include "Stockpile.h"
#include <QtGui>
#include <vector>
#include <boost/shared_ptr.hpp>

class StockpileNetwork : public QObject
{
    Q_OBJECT

    public:

        StockpileNetwork();

        void addStockpile(boost::shared_ptr<Stockpile> stockpile);

        std::vector<boost::shared_ptr<Stockpile> > getStockpiles();

    signals:

        void changed();

    private:

        std::vector<boost::shared_ptr<Stockpile> > stockpiles_;
};

#endif
