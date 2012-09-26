#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

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

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        void setTime(int time);

    private:

        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;

        QRectF viewRect_;

        boost::shared_ptr<QSvgRenderer> baseMapSvg_;
        QRectF baseMapRect_;
        GLuint baseMapTextureId_;
        bool baseMapTextureBound_;

        std::map<int, boost::shared_ptr<MapShape> > counties_;

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
