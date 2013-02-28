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

void MapShape::renderBoundary()
{
    glPushAttrib(GL_CURRENT_BIT);

    glColor4f(0.,0.,0.,1.);

    glPushMatrix();
    glTranslatef(0.,0.,0.2);

    glBegin(GL_LINE_STRIP);

    for(unsigned int i=0; i<vertices_.size(); i++)
    {
        glVertex2d(vertices_[i].lon, vertices_[i].lat);
    }

    glEnd();

    glPopMatrix();
    glPopAttrib();
}

void MapShape::renderFilled()
{
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(r_, g_, b_, 0.5);

    glPushMatrix();
    glTranslatef(0.,0.,0.1);

    glBegin(GL_POLYGON);

    for(unsigned int i=0; i<vertices_.size(); i++)
    {
        glVertex2d(vertices_[i].lon, vertices_[i].lat);
    }

    glEnd();

    glPopMatrix();
    glPopAttrib();
}

void MapShape::renderSVG(QPainter * painter)
{
    QVector<QPointF> points;

    for(unsigned int i=0; i<vertices_.size(); i++)
    {
        points.push_back(QPointF(vertices_[i].lon, vertices_[i].lat));
    }

    QPolygonF polygon(points);

    painter->setBrush(QBrush(QColor::fromRgbF(r_ , g_, b_, 1.)));
    painter->setPen(QPen(QBrush(), .01));
    painter->drawPolygon(polygon);
}
