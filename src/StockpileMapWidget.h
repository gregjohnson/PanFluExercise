#ifndef STOCKPILE_MAP_WIDGET_H
#define STOCKPILE_MAP_WIDGET_H

#include "MapWidget.h"
#include "Stockpile.h"
#include <boost/shared_ptr.hpp>

class StockpileNetwork;

class StockpileMapWidget : public MapWidget
{
    public:

        StockpileMapWidget();

        // re-implemented virtual methods
        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        void setTime(int time);

        void setType(STOCKPILE_TYPE type);

    private:

        boost::shared_ptr<StockpileNetwork> stockpileNetwork_;

        STOCKPILE_TYPE type_;

        // re-implemented virtual render method
        void render(QPainter * painter);
};

#endif
