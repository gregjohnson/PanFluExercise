#ifndef MAP_SHAPE_H
#define MAP_SHAPE_H

#include <vector>

struct MapVertex {
    double lat;
    double lon;
};

class MapShape
{
    public:

        MapShape();
        ~MapShape();

        void addVertex(double lat, double lon);

        void setColor(float r, float g, float b);

        void render();

    private:

        std::vector<MapVertex> vertices_;

        float r_, g_, b_;
};

#endif
