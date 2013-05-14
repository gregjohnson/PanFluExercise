#include "MapWidget.h"
#include "MapShape.h"
#include "EpidemicDataSet.h"
#include "main.h"
#include "log.h"
#include <QtOpenGL>
#include <string>
#include <ogrsf_frmts.h>

#ifdef __APPLE__
    #include <OpenGL/glu.h>
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
    baseMapTextureBound_ = false;

    // load base map
    std::string svgFilename = g_dataDirectory + "/basemap.svg";

    if(loadBaseMapSvg(svgFilename.c_str()) != true)
    {
        put_flog(LOG_FATAL, "could not load base map %s", svgFilename.c_str());
        exit(1);
    }

    // load county shapes
    if(loadCountyShapes() != true)
    {
        put_flog(LOG_FATAL, "could not load county shapes");
        exit(1);
    }

    // default counties color map
    countiesColorMap_.setColorMap(0., 1.);
}

MapWidget::~MapWidget()
{
    // delete current base map texture
    if(baseMapTextureBound_ == true)
    {
        deleteTexture(baseMapTextureId_);
    }
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

void MapWidget::exportSVGToDisplayCluster()
{
    if(g_dcSocket != NULL && svgTmpFile_.open())
    {
        QSvgGenerator generator;
        generator.setFileName(svgTmpFile_.fileName());
        generator.setSize(QSize(1400, 1200));
        generator.setViewBox(baseMapRect_);

        QPainter painter;
        painter.begin(&generator);

        // set logical coordinates of the render window
        painter.setWindow(baseMapRect_.toRect());

        // draw a black background
        painter.setBrush(QBrush(QColor::fromRgbF(0,0,0,1)));
        painter.drawRect(QRect(QPoint(-107,37), QPoint(-93,25)));

        // don't render the basemap in SVG since it's too expensive
        // baseMapSvg_->render(&painter, QRect(QPoint(-107,37), QPoint(-93,25)));

        // render shapes
        std::map<int, boost::shared_ptr<MapShape> >::iterator iter;

        for(iter=counties_.begin(); iter!=counties_.end(); iter++)
        {
            iter->second->renderSVG(&painter);
        }

        // draw title
        painter.resetTransform();

        QFont titleFont = painter.font();

        int titleFontPixelSize = 32;
        titleFont.setPixelSize(titleFontPixelSize);

        painter.setFont(titleFont);
        painter.setPen(QColor::fromRgbF(1,1,1,1));
        painter.setBrush(QBrush(QColor::fromRgbF(1,1,1,1)));

        QPoint titlePosition = painter.window().topRight() + QPoint(-600, 100);
        QRect titleRect = QRect(titlePosition, titlePosition + QPoint(600, 3 * titleFontPixelSize));

        painter.drawText(titleRect, Qt::TextWordWrap, title_.c_str(), &titleRect);

        // draw legend
        QPointF legendTopLeft = painter.window().bottomRight() + QPointF(-250, -250);
        float legendHeight = 200.;
        float legendWidth = 50.;

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

            painter.setPen(QColor::fromRgbF(r,g,b,1));
            painter.setBrush(QBrush(QColor::fromRgbF(r,g,b,1)));

            painter.drawRect(QRectF(tl, br));
        }

        // draw legend labels
        float textPadding = 25;

        QPointF legendMax = QPointF(legendTopLeft) + QPointF(legendWidth + textPadding, titleFontPixelSize);
        QPointF legendMin = QPointF(legendTopLeft) + QPointF(legendWidth + textPadding, legendHeight);

        painter.setPen(QColor::fromRgbF(1,1,1,1));
        painter.setBrush(QBrush(QColor::fromRgbF(1,1,1,1)));

        painter.drawText(legendMax, colorMapMaxLabel_.c_str());
        painter.drawText(legendMin, colorMapMinLabel_.c_str());

        painter.end();

        put_flog(LOG_DEBUG, "wrote %s", svgTmpFile_.fileName().toStdString().c_str());

        // now, send it to DisplayCluster
        sendSVGToDisplayCluster((svgTmpFile_.fileName()).toStdString(), (QString("ExerciseMap-") + QString::number(index_) + ".svg").toStdString());
    }
}

void MapWidget::initializeGL()
{
    // enable depth testing; disable lighting
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
}

void MapWidget::paintGL()
{
    setOrthographicView();

    renderBaseMapTexture();

    // other derived class rendering
    render();
}

void MapWidget::resizeGL(int width, int height)
{
    // generate new base map texture of appropriate resolution on window resize
    generateBaseMapTexture(width, height);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    update();
}

void MapWidget::setOrthographicView()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(viewRect_.left(), viewRect_.right(), viewRect_.bottom(), viewRect_.top());
    glPushMatrix();

    glMatrixMode(GL_MODELVIEW); 
    glLoadIdentity();

    glClearColor(0,0,0,0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool MapWidget::loadBaseMapSvg(const char * filename)
{
    boost::shared_ptr<QSvgRenderer> baseMapSvg(new QSvgRenderer(QString(filename)));
    baseMapSvg_ = baseMapSvg;

    baseMapRect_ = QRectF(QPointF(-107.,37.), QPointF(-93.,25.));

    viewRect_ = baseMapRect_;

    return baseMapSvg_->isValid();
}

void MapWidget::generateBaseMapTexture(int width, int height)
{
    // delete current base map texture
    if(baseMapTextureBound_ == true)
    {
        deleteTexture(baseMapTextureId_);
    }

    // generate new texture
    QImage image(width, height, QImage::Format_ARGB32);
    QPainter painter(&image);
    baseMapSvg_->render(&painter);

    baseMapTextureId_ = bindTexture(image, GL_TEXTURE_2D, GL_RGBA, QGLContext::DefaultBindOption);
    baseMapTextureBound_ = true;
}

void MapWidget::renderBaseMapTexture()
{
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);

    glColor4f(1,1,1,1);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, baseMapTextureId_);

    glBegin(GL_QUADS);

    glTexCoord2f(0.,0.);
    glVertex2f(baseMapRect_.left(), baseMapRect_.bottom());

    glTexCoord2f(1.,0.);
    glVertex2f(baseMapRect_.right(), baseMapRect_.bottom());

    glTexCoord2f(1.,1.);
    glVertex2f(baseMapRect_.right(), baseMapRect_.top());

    glTexCoord2f(0.,1.);
    glVertex2f(baseMapRect_.left(), baseMapRect_.top());

    glEnd();

    glPopAttrib();
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

        OGRGeometry * geometry = feature->GetGeometryRef()->Simplify(0.01);

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

void MapWidget::renderCountyShapes()
{
    std::map<int, boost::shared_ptr<MapShape> >::iterator iter;

    for(iter=counties_.begin(); iter!=counties_.end(); iter++)
    {
        iter->second->renderBoundary();
    }

    for(iter=counties_.begin(); iter!=counties_.end(); iter++)
    {
        iter->second->renderFilled();
    }
}
