#ifndef STOCKPILE_WIDGET_H
#define STOCKPILE_WIDGET_H

#include <QtGui>
#include <boost/shared_ptr.hpp>

class Stockpile;

class StockpileWidget : public QGroupBox
{
    public:

        StockpileWidget(boost::shared_ptr<Stockpile> stockpile);

    private:

        boost::shared_ptr<Stockpile> stockpile_;

        // UI elements
        QSpinBox numSpinBox_;
};

#endif
