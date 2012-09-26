#include "MapShape.h"
#include <QtOpenGL>

MapShape::MapShape()
{
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

void MapShape::setColor(float r, float g, float b)
{
    r_ = r;
    g_ = g;
    b_ = b;
}

void MapShape::render()
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
