#ifndef COLOR_MAP_H
#define COLOR_MAP_H

#include <vector>

struct Color {

    Color(float red, float green, float blue)
    {
        r = red;
        g = green;
        b = blue;
    }

    float r;
    float g;
    float b;
};

class ColorMap {

    public:

        ColorMap();

        void setColorVector(std::vector<Color> colorVector);

        // get and set color / alpha map entries
        void getColorMap(float &colorMin, float &colorMax);
        void setColorMap(float colorMin, float colorMax);

        void getAlphaMap(float &alphaMin, float &alphaMax);
        void setAlphaMap(float alphaMin, float alphaMax);

        // set colors directly according to the maps
        void setColor3(float value);
        void setColor4(float value);

        // functions for getting colors
        void getColor3(float value, float &r, float &g, float &b);
        void getColor4(float value, float &r, float &g, float &b, float &a);

        // this actually maps an index to a color
        void mapIndex(float index, float &r, float &g, float &b);

    private:


        std::vector<Color> colorVector_;

        float colorMin_;
        float colorMax_;

        float alphaMin_;
        float alphaMax_;
};

#endif
