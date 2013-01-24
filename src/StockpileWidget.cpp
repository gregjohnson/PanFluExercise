#include "StockpileWidget.h"
#include "Stockpile.h"

StockpileWidget::StockpileWidget(boost::shared_ptr<Stockpile> stockpile)
{
    setTitle(stockpile->getName().c_str());

    QVBoxLayout * layout = new QVBoxLayout();
    setLayout(layout);

    // add num cases widget
    numSpinBox_.setMaximum(9999999);
    layout->addWidget(&numSpinBox_);

    // current value
    numSpinBox_.setValue(stockpile->getNum());

    // connections
    connect(&numSpinBox_, SIGNAL(valueChanged(int)), stockpile.get(), SLOT(setNum(int)));

    // todo: connection with stockpile changed() signal to update UI elements here
}
