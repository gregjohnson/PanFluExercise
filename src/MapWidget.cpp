#include "MapWidget.h"
#include "config.h"
#include "log.h"
#include <QtOpenGL>
#include <string>

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
}

MapWidget::~MapWidget()
{
    // delete current base map texture
    if(baseMapTextureBound_ == true)
    {
        deleteTexture(baseMapTextureId_);
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
    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);

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
