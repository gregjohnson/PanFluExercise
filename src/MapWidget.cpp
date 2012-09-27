#include "MapWidget.h"
#include "MapShape.h"
#include "EpidemicDataSet.h"
#include "config.h"
#include "log.h"
#include <QtOpenGL>
#include <string>
#include <ogrsf_frmts.h>

#ifdef __APPLE__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

MapWidget::MapWidget()
{
    // defaults
    baseMapTextureBound_ = false;

    // load base map
    std::string svgFilename = std::string(ROOT_DIRECTORY) + "/data/basemap.svg";

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
    countiesColorMap_.setColorMap(0., 0.01);
}

MapWidget::~MapWidget()
{
    // delete current base map texture
    if(baseMapTextureBound_ == true)
    {
        deleteTexture(baseMapTextureId_);
    }
}

void MapWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;
}

void MapWidget::setTime(int time)
{
    time_ = time;

    // recolor counties
    if(dataSet_ != NULL)
    {
        std::map<int, boost::shared_ptr<MapShape> >::iterator iter;

        for(iter=counties_.begin(); iter!=counties_.end(); iter++)
        {
            // get total infected
            float infectedFraction = dataSet_->getValue("infected", time_, iter->first) / dataSet_->getPopulation(iter->first);

            // map to color
            float r, g, b;
            countiesColorMap_.getColor3(infectedFraction, r, g, b);

            iter->second->setColor(r, g, b);
        }

        // force redraw
        update();
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
    renderCountyShapes();
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

    std::string filename = std::string(ROOT_DIRECTORY) + "/data/counties/tl_2009_48_county00.shp";

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
        int fipsId = feature->GetFieldAsInteger("COUNTYFP00");

        if(fipsId == 0)
        {
            put_flog(LOG_WARN, "invalid county");
        }

        // add a new county to the counties map corresponding to this fipsId
        boost::shared_ptr<MapShape> county(new MapShape());
        counties_[fipsId] = county;

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
