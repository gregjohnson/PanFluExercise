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

MapWidget::MapWidget()
{
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
            // get total infectious
            float infectiousFraction = dataSet_->getValue("infectious", time_, iter->first) / dataSet_->getPopulation(iter->first);

            // map to color
            float r, g, b;
            countiesColorMap_.getColor3(infectiousFraction, r, g, b);

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
    renderCountyTravel();
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

void MapWidget::renderCountyTravel()
{
    // parameters
    float infectiousTravelerThreshhold = 1.;
    float infectiousTravelerAlphaScale = 100.;

    if(dataSet_ == NULL)
    {
        return;
    }

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(3.);

    glBegin(GL_LINES);

    for(std::map<int, boost::shared_ptr<MapShape> >::iterator iter0=counties_.begin(); iter0!=counties_.end(); iter0++)
    {
        int nodeId0 = iter0->first;

        double lat0, lon0;
        iter0->second->getCentroid(lat0, lon0);

        // get number of infectious in node0
        float infectiousNode0 = dataSet_->getValue("infectious", time_, nodeId0);

        if(infectiousNode0 < infectiousTravelerThreshhold)
        {
            continue;
        }

        for(std::map<int, boost::shared_ptr<MapShape> >::iterator iter1=counties_.begin(); iter1!=counties_.end(); iter1++)
        {
            int nodeId1 = iter1->first;

            double lat1, lon1;
            iter1->second->getCentroid(lat1, lon1);

            if(nodeId0 != nodeId1)
            {
                float travel = dataSet_->getTravel(nodeId0, nodeId1);

                float infectiousTravelers = infectiousNode0 * travel;

                if(infectiousTravelers > infectiousTravelerThreshhold)
                {
                    glColor4f(1.,0.,0., infectiousTravelers / infectiousTravelerAlphaScale);

                    glVertex2f(lon0, lat0);
                    glVertex2f(lon1, lat1);
                }
            }
        }
    }

    glEnd();

    glPopAttrib();
}
