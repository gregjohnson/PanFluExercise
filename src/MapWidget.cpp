#include "main.h"
#include "MapWidget.h"
#include "MapShape.h"
#include "EpidemicDataSet.h"
#include "log.h"
#include <QtOpenGL>
#include <string>
#include <ogrsf_frmts.h>

#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#elif WIN32
    #include <windows.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#else
    #include <GL/glu.h>
#endif

int MapWidget::numMapWidgets_ = 0;

MapWidget::MapWidget()
{
    // indexing
    index_ = numMapWidgets_;
    numMapWidgets_++;

    // defaults
    viewRect_ = QRectF(QPointF(-107.,37.), QPointF(-93.,25.));

    // load county shapes
    if(loadCountyShapes() != true)
    {
        put_flog(LOG_FATAL, "could not load county shapes");
        exit(1);
    }

    // default counties color map
    countiesColorMap_.setColorMap(0., 1.);

    // for antialiasing
    QGLWidget::setFormat(QGLFormat(QGL::SampleBuffers));
}

MapWidget::~MapWidget()
{

}

void MapWidget::setTitle(std::string title)
{
    title_ = title;
}

void MapWidget::setColorMapMinLabel(std::string label)
{
    colorMapMinLabel_ = label;
}

void MapWidget::setColorMapMaxLabel(std::string label)
{
    colorMapMaxLabel_ = label;
}

void MapWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;
}

void MapWidget::setTime(int time)
{
    time_ = time;
}

#if USE_DISPLAYCLUSTER
void MapWidget::exportSVGToDisplayCluster()
{
    if(g_dcSocket != NULL && svgTmpFile_.open())
    {
        QSvgGenerator generator;
        generator.setFileName(svgTmpFile_.fileName());
        generator.setResolution(90);
        generator.setSize(QSize(1400, 1200));
        generator.setViewBox(viewRect_);

        QPainter painter;
        painter.begin(&generator);

        // set logical coordinates of the render window
        painter.setWindow(viewRect_.toRect());

        // draw a black background
        painter.setBrush(QBrush(QColor::fromRgbF(0,0,0,1)));
        painter.drawRect(QRect(QPoint(-107,37), QPoint(-93,25)));

        renderAll(&painter, false);

        painter.end();

        put_flog(LOG_DEBUG, "wrote %s", svgTmpFile_.fileName().toStdString().c_str());

        // now, send it to DisplayCluster
        sendSVGToDisplayCluster((svgTmpFile_.fileName()).toStdString(), (QString("ExerciseMap-") + QString::number(index_) + ".svg").toStdString());
    }
}
#endif

void MapWidget::renderAll(QPainter * painter, bool uiRender)
{
    // derived class rendering
    render(painter);

    // draw title
    painter->resetTransform();

    QFont titleFont = painter->font();

    int titleFontPixelSize;

    if (uiRender)
        titleFontPixelSize = 16;
    else
        titleFontPixelSize = 32;

    QColor titleColor;

    if (uiRender)
    {
        titleColor = QColor::fromRgbF(.1,.1,.1,1);
    }
    else
    {
        titleColor = QColor::fromRgbF(1,1,1,1);
    }

    titleFont.setPixelSize(titleFontPixelSize);

    painter->setFont(titleFont);
    painter->setPen(titleColor);
    painter->setBrush(QBrush(titleColor));

    QPoint titlePosition;
    QRect titleRect;

    if(uiRender)
    {
        titlePosition = painter->window().topLeft() + QPoint(painter->window().width() * 1./4., .1 * titleFontPixelSize);
        titleRect = QRect(titlePosition, titlePosition + QPoint(250, 1.0 * titleFontPixelSize));
        painter->drawText(titleRect, Qt::TextWordWrap, title_.c_str(), &titleRect);
    }
    else
    {
        titlePosition = painter->window().topRight() + QPoint(-600, 100);
        titleRect = QRect(titlePosition, titlePosition + QPoint(600, 3 * titleFontPixelSize));
        painter->drawText(titleRect, Qt::TextWordWrap, title_.c_str(), &titleRect);
    }


    // draw legend
    QPointF legendTopLeft;
    float legendHeight, legendWidth;
    float textPadding;

    if(uiRender)
    {
        legendTopLeft = painter->window().bottomRight() + QPointF(-60, -90);
        legendHeight = 80.;
        legendWidth = 10.;
        textPadding = 10;
    }
    else
    {
        legendTopLeft = painter->window().bottomRight() + QPointF(-250, -250);
        legendHeight = 200.;
        legendWidth = 50.;
        textPadding = 25;
    }

    int legendSubdivisions = 50;

    float colorMin, colorMax;
    countiesColorMap_.getColorMap(colorMin, colorMax);

    for(int i=0; i<legendSubdivisions; i++)
    {
        QPointF tl = QPointF(legendTopLeft) + QPointF(0., (float)i * legendHeight / (float)legendSubdivisions);
        QPointF br = tl + QPointF(legendWidth, legendHeight / (float)legendSubdivisions);

        float value = colorMax + (float)i/((float)legendSubdivisions - 1.) * (colorMin - colorMax);

        float r,g,b;
        countiesColorMap_.getColor3(value, r,g,b);

        painter->setPen(QColor::fromRgbF(r,g,b,1));
        painter->setBrush(QBrush(QColor::fromRgbF(r,g,b,1)));

        painter->drawRect(QRectF(tl, br));
    }

    // draw legend labels
    QPointF legendMax = QPointF(legendTopLeft) + QPointF(legendWidth + textPadding, titleFontPixelSize);
    QPointF legendMin = QPointF(legendTopLeft) + QPointF(legendWidth + textPadding, legendHeight);

    painter->setPen(titleColor);
    painter->setBrush(QBrush(titleColor));

    painter->drawText(legendMax, colorMapMaxLabel_.c_str());
    painter->drawText(legendMin, colorMapMinLabel_.c_str());
}

void MapWidget::paintEvent(QPaintEvent* event)
{
    makeCurrent();

    // for antialiasing
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);

    QPainter painter(this);

    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    painter.setWindow(viewRect_.toRect());

    renderAll(&painter, true);
}

void MapWidget::resizeEvent(QResizeEvent * event)
{
    makeCurrent();

    int width = event->size().width();
    int height = event->size().height();

    update();
}

bool MapWidget::loadCountyShapes()
{
    OGRRegisterAll();

    std::string filename = g_dataDirectory + "/counties/tl_2009_48_county00.shp";

    OGRDataSource * dataSource = OGRSFDriverRegistrar::Open(filename.c_str(), false);

    if(dataSource == NULL)
    {
        put_flog(LOG_ERROR, "could not open %s", filename.c_str());
        return false;
    }

    OGRLayer * layer = dataSource->GetLayerByName("tl_2009_48_county00");

    layer->ResetReading();

    OGRFeature * feature;

    while((feature = layer->GetNextFeature()) != NULL)
    {
        // get county FIPS code
        int nodeId = feature->GetFieldAsInteger("COUNTYFP00");

        if(nodeId == 0)
        {
            put_flog(LOG_WARN, "invalid county");
        }

        // add a new county to the counties map corresponding to this nodeId
        boost::shared_ptr<MapShape> county(new MapShape());
        counties_[nodeId] = county;

        OGRGeometry * geometry = feature->GetGeometryRef();

        if(geometry != NULL && geometry->getGeometryType() == wkbPolygon)
        {
            OGRPolygon * polygon = (OGRPolygon *)geometry;

            OGRLinearRing * ring = polygon->getExteriorRing();

            for(int i=0; i<ring->getNumPoints(); i++)
            {
                // x is longitude, y latitude
                county->addVertex(ring->getY(i), ring->getX(i));
            }

            // set the centroid
            OGRPoint centroidPoint;

            if(polygon->Centroid(&centroidPoint) == OGRERR_NONE)
            {
                county->setCentroid(centroidPoint.getY(), centroidPoint.getX());
            }
            else
            {
                put_flog(LOG_WARN, "no polygon centroid");
            }
        }
        else
        {
            put_flog(LOG_WARN, "no polygon geometry");
        }

        OGRFeature::DestroyFeature(feature);
    }

    OGRDataSource::DestroyDataSource(dataSource);

    return true;
}

void MapWidget::renderCountyShapes(QPainter * painter)
{
    std::map<int, boost::shared_ptr<MapShape> >::iterator iter;

    for(iter=counties_.begin(); iter!=counties_.end(); iter++)
    {
        iter->second->render(painter);
    }
}
