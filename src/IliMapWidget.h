#ifndef ILI_MAP_WIDGET_H
#define ILI_MAP_WIDGET_H

#include "MapWidget.h"

class IliMapWidget : public MapWidget
{
    public:

        IliMapWidget();

        // re-implemented virtual methods
        void setTime(int time);

    private:

        // re-implemented virtual render method
        void render(QPainter * painter);
};

#endif
