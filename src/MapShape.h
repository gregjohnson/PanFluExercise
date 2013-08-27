#ifndef MAP_SHAPE_H
#define MAP_SHAPE_H

#include <vector>

struct MapVertex {
    double lat;
    double lon;
};

class QPainter;

class MapShape
{
    public:

        MapShape();
        ~MapShape();

        void addVertex(double lat, double lon);

        void setCentroid(double lat, double lon);
        void getCentroid(double &lat, double &lon);

        void setColor(float r, float g, float b);

        void render(QPainter * painter);

    private:

        std::vector<MapVertex> vertices_;

        // centroid; must be set, is not computed automatically
        double centroidLat_, centroidLon_;

        float r_, g_, b_;
};

#endif
