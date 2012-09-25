#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

#include <QGLWidget>
#include <QtSvg>
#include <boost/shared_ptr.hpp>

class MapWidget : public QGLWidget
{
    public:

        MapWidget();
        ~MapWidget();

    private:

        QRectF viewRect_;

        boost::shared_ptr<QSvgRenderer> baseMapSvg_;
        QRectF baseMapRect_;
        GLuint baseMapTextureId_;
        bool baseMapTextureBound_;

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

};

#endif
