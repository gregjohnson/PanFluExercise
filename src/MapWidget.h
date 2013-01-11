#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

#include "ColorMap.h"
#include <QGLWidget>
#include <QtSvg>
#include <boost/shared_ptr.hpp>
#include <map>

class EpidemicDataSet;
class MapShape;

class MapWidget : public QGLWidget
{
    Q_OBJECT

    public:

        MapWidget();
        ~MapWidget();

    public slots:

        virtual void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        virtual void setTime(int time);

    protected:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;

        // window view rectangle
        QRectF viewRect_;

        // base map information
        boost::shared_ptr<QSvgRenderer> baseMapSvg_;
        QRectF baseMapRect_;
        GLuint baseMapTextureId_;
        bool baseMapTextureBound_;

        // county shapes
        std::map<int, boost::shared_ptr<MapShape> > counties_;

        // color map for county shapes
        ColorMap countiesColorMap_;

        // render() method placeholder for derived classes
        virtual void render() { }

        // reimplemented from QGLWidget
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

        // setup basic orthographic view
        void setOrthographicView();

        // base map
        bool loadBaseMapSvg(const char * filename);
        void generateBaseMapTexture(int width, int height);
        void renderBaseMapTexture();

        // counties
        bool loadCountyShapes();
        void renderCountyShapes();
};

#endif
