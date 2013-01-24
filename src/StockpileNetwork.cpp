#include "StockpileNetwork.h"

StockpileNetwork::StockpileNetwork()
{

}

void StockpileNetwork::addStockpile(boost::shared_ptr<Stockpile> stockpile)
{
    connect(stockpile.get(), SIGNAL(changed()), this, SIGNAL(changed()));

    stockpiles_.push_back(stockpile);

    emit(changed());
}

std::vector<boost::shared_ptr<Stockpile> > StockpileNetwork::getStockpiles()
{
    return stockpiles_;
}
