#include "ColorMap.h"
#include <cmath>

#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

ColorMap::ColorMap()
{
    // color map definition (CFD-type)
    colorArray[0][0] = 0.;
    colorArray[0][1] = 1.;
    colorArray[0][2] = 0.;

    colorArray[1][0] = 1.;
    colorArray[1][1] = 1.;
    colorArray[1][2] = 0.;

    colorArray[2][0] = 1.;
    colorArray[2][1] = 0.;
    colorArray[2][2] = 0.;

    // defaults
    colorMin_ = colorMax_ = 0.;
    alphaMin_ = alphaMax_ = 0.;
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
    if(index <= 0.)
    {
        r = colorArray[0][0];
        g = colorArray[0][1];
        b = colorArray[0][2];

        return;
    }
    else if(index >= 1.)
    {
        r = colorArray[COLOR_ARRAY_SIZE-1][0];
        g = colorArray[COLOR_ARRAY_SIZE-1][1];
        b = colorArray[COLOR_ARRAY_SIZE-1][2];

        return;
    }

    // index is in range, let's interpolate a color
    index = index * ((float)COLOR_ARRAY_SIZE - 1.);

    // interpolate between the two bounding colors
    r = colorArray[(int)index][0] + (index - floor(index)) * (colorArray[(int)index+1][0] - colorArray[(int)index][0]);
    g = colorArray[(int)index][1] + (index - floor(index)) * (colorArray[(int)index+1][1] - colorArray[(int)index][1]);
    b = colorArray[(int)index][2] + (index - floor(index)) * (colorArray[(int)index+1][2] - colorArray[(int)index][2]);
}
