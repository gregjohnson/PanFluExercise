#ifndef EPIDEMIC_MAP_WIDGET_H
#define EPIDEMIC_MAP_WIDGET_H

#include "MapWidget.h"

class EpidemicMapWidget : public MapWidget
{
    public:

        EpidemicMapWidget();

        // re-implemented virtual methods
        void setTime(int time);

    private:

        // re-implemented virtual render method
        void render(QPainter * painter);

        // travel between counties
        void renderCountyTravel(QPainter * painter);
};

#endif
