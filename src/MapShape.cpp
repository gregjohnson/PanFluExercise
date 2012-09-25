#include "MapShape.h"
#include <QtOpenGL>

MapShape::MapShape()
{
    
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

void MapShape::render()
{
    glPushAttrib(GL_CURRENT_BIT);

    glColor4f(1,0,0,1);

    glPushMatrix();
    glTranslatef(0.,0.,0.1);

    glBegin(GL_LINE_STRIP);

    for(unsigned int i=0; i<vertices_.size(); i++)
    {
        glVertex2d(vertices_[i].lon, vertices_[i].lat);
    }

    glEnd();

    glPopMatrix();
    glPopAttrib();
}
