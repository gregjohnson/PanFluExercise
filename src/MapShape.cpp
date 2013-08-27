#include "MapShape.h"
#include <QtOpenGL>
#include <QtGui>

MapShape::MapShape()
{
    // defaults
    centroidLat_ = centroidLon_ = 0.;
    r_ = g_ = b_ = 1.;
}

MapShape::~MapShape()
{
    
}

void MapShape::addVertex(double lat, double lon)
{
    MapVertex v;
    v.lat = lat;
    v.lon = lon;

    vertices_.push_back(v);
}

void MapShape::setCentroid(double lat, double lon)
{
    centroidLat_ = lat;
    centroidLon_ = lon;
}

void MapShape::getCentroid(double &lat, double &lon)
{
    lat = centroidLat_;
    lon = centroidLon_;
}

void MapShape::setColor(float r, float g, float b)
{
    r_ = r;
    g_ = g;
    b_ = b;
}

void MapShape::render(QPainter * painter)
{
    QVector<QPointF> points;

    for(unsigned int i=0; i<vertices_.size(); i++)
    {
        points.push_back(QPointF(vertices_[i].lon, vertices_[i].lat));
    }

    QPolygonF polygon(points);

    painter->setBrush(QBrush(QColor::fromRgbF(r_ , g_, b_, 1.)));
    painter->setPen(QPen(QBrush(QColor::fromRgbF(.5, .5, .5, 1.)), .03));

    painter->drawPolygon(polygon);
}
