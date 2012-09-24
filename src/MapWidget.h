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

        boost::shared_ptr<QSvgRenderer> baseMapSvg_;
        GLuint baseMapTextureId_;
        bool baseMapTextureBound_;

        // reimplemented from QGLWidget
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

        void setOrthographicView();

        bool loadBaseMapSvg(QString filename);
        void generateBaseMapTexture(int width, int height);
        void renderBaseMapTexture();

};

#endif
