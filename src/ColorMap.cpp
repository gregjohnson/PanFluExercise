#include "ColorMap.h"
#include "log.h"
#include <cmath>

#ifdef __APPLE__
    #include <OpenGL/gl.h>
#elif WIN32
    #include <windows.h>
    #include <GL/gl.h>
#else
    #include <GL/gl.h>
#endif

ColorMap::ColorMap()
{
    // color map definition (CFD-type)
    colorVector_.push_back(Color(0.,1.,0.));
    colorVector_.push_back(Color(1.,1.,0.));
    colorVector_.push_back(Color(1.,0.,0.));

    // defaults
    colorMin_ = colorMax_ = 0.;
    alphaMin_ = alphaMax_ = 0.;
}

void ColorMap::setColorVector(std::vector<Color> colorVector)
{
    colorVector_ = colorVector;
}

void ColorMap::getColorMap(float &colorMin, float &colorMax)
{
    colorMin = colorMin_;
    colorMax = colorMax_;
}

void ColorMap::setColorMap(float colorMin, float colorMax)
{
    colorMin_ = colorMin;
    colorMax_ = colorMax;
}

void ColorMap::getAlphaMap(float &alphaMin, float &alphaMax)
{
    alphaMin = alphaMin_;
    alphaMax = alphaMax_;
}

void ColorMap::setAlphaMap(float alphaMin, float alphaMax)
{
    alphaMin_ = alphaMin;
    alphaMax_ = alphaMax;
}

void ColorMap::setColor3(float value)
{
    float r,g,b;
    getColor3(value, r, g, b);	

    glColor3f(r, g, b);
}

void ColorMap::setColor4(float value)
{
    float r,g,b,a;
    getColor4(value, r,g,b,a);

    glColor4f(r, g, b, a);
}

void ColorMap::getColor3(float value, float &r, float &g, float &b)
{
    // if map is not defined, default to white
    if(colorMin_ == 0. && colorMax_ == 0.)
    {
        r = g = b = 1.;
        return;
    }

    float index = (value - colorMin_) / (colorMax_ - colorMin_);

    mapIndex(index, r, g, b);
}

void ColorMap::getColor4(float value, float &r, float &g, float &b, float &a)
{
    getColor3(value, r, g, b);

    // if map is not defined, default to opaque
    if(alphaMin_ == 0. && alphaMax_ == 0.)
    {
        a = 1.;
        return;
    }

    a = (value - alphaMin_) / (alphaMax_ - alphaMin_);

    // clamp between 0 and 1
    if(a < 0.)
        a = 0.;
    else if(a > 1.)
        a = 1.;
}

void ColorMap::mapIndex(float index, float &r, float &g, float &b)
{
    if(colorVector_.size() <= 0)
    {
        put_flog(LOG_ERROR, "undefined color vector");
        r = g = b = 1.;

        return;
    }

    if(index <= 0.)
    {
        r = colorVector_[0].r;
        g = colorVector_[0].g;
        b = colorVector_[0].b;

        return;
    }
    else if(index >= 1.)
    {
        r = colorVector_[colorVector_.size()-1].r;
        g = colorVector_[colorVector_.size()-1].g;
        b = colorVector_[colorVector_.size()-1].b;

        return;
    }

    // index is in range, let's interpolate a color
    index = index * ((float)colorVector_.size() - 1.);

    // interpolate between the two bounding colors
    r = colorVector_[(int)index].r + (index - floor(index)) * (colorVector_[(int)index+1].r - colorVector_[(int)index].r);
    g = colorVector_[(int)index].g + (index - floor(index)) * (colorVector_[(int)index+1].g - colorVector_[(int)index].g);
    b = colorVector_[(int)index].b + (index - floor(index)) * (colorVector_[(int)index+1].b - colorVector_[(int)index].b);
}
