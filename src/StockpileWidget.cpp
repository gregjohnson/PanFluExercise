#include "StockpileWidget.h"
#include "Stockpile.h"

StockpileWidget::StockpileWidget(boost::shared_ptr<Stockpile> stockpile)
{
    stockpile_ = stockpile;

    setTitle(stockpile->getName().c_str());

    QVBoxLayout * layout = new QVBoxLayout();
    setLayout(layout);

    // add num cases label
    layout->addWidget(&numLabel_);

    // current value
    numLabel_.setNum(stockpile->getNum(0));
}

void StockpileWidget::setTime(int time)
{
    // update UI element
    numLabel_.setNum(stockpile_->getNum(time));
}
