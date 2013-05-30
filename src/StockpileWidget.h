#ifndef STOCKPILE_WIDGET_H
#define STOCKPILE_WIDGET_H

#define STOCKPILE_WIDGET_NUM_MAX 99999999

#include <QtGui>
#include <boost/shared_ptr.hpp>

class Stockpile;

class StockpileWidget : public QGroupBox
{
    public:

        StockpileWidget(boost::shared_ptr<Stockpile> stockpile);

        void setTime(int time);

    private:

        boost::shared_ptr<Stockpile> stockpile_;

        // UI elements

};

#endif
