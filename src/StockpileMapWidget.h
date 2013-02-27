#ifndef STOCKPILE_MAP_WIDGET_H
#define STOCKPILE_MAP_WIDGET_H

#include "MapWidget.h"
#include <boost/shared_ptr.hpp>

class StockpileNetwork;

class StockpileMapWidget : public MapWidget
{
    public:

        // re-implemented virtual methods
        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        void setTime(int time);

    private:

        boost::shared_ptr<StockpileNetwork> stockpileNetwork_;

        // re-implemented virtual render method
        void render();
};

#endif
