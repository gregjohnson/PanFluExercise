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

        void setTitle(std::string title);

        void setColorMapMinLabel(std::string label);
        void setColorMapMaxLabel(std::string label);

    public slots:

        virtual void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        virtual void setTime(int time);

#if USE_DISPLAYCLUSTER
        void exportSVGToDisplayCluster();
#endif

    protected:

        // indexing used to identify this map
        static int numMapWidgets_;
        int index_;

        // window view rectangle
        QRectF viewRect_;

        // title and color map labels
        std::string title_;

        std::string colorMapMinLabel_;
        std::string colorMapMaxLabel_;

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;

        // county shapes
        std::map<int, boost::shared_ptr<MapShape> > counties_;

        // color map for county shapes
        ColorMap countiesColorMap_;

        // SVG export
        QTemporaryFile svgTmpFile_;

        // render() method placeholder for derived classes
        virtual void render(QPainter * painter) { }

        void renderAll(QPainter * painter, bool uiRender);

        // reimplemented from QGLWidget
        void paintEvent(QPaintEvent* event);
        void resizeEvent(QResizeEvent * event);

        // counties
        bool loadCountyShapes();
        void renderCountyShapes(QPainter * painter);
};

#endif
