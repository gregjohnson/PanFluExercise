#ifndef TIMELINE_WIDGET_H
#define TIMELINE_WIDGET_H

#include <QtGui>
#include <boost/shared_ptr.hpp>
#include <QGLWidget>

class MainWindow;
class EventMonitor;
struct EventMessage;

class TimelineWidget : public QGLWidget
{
    Q_OBJECT

    public:

        TimelineWidget(MainWindow* mainWindow, EventMonitor* monitor);
        ~TimelineWidget();

        void exportSVGToDisplayCluster();
        void render(QPainter* painter);

    public slots:

        void clearMessages();
        void insertEventMessage(boost::shared_ptr<EventMessage> message);
        void paintEvent(QPaintEvent* event);
        void setTime(int time);

    private:

        int time_;

        EventMonitor* monitor_;

        // SVG export
        QTemporaryFile svgTmpFile_;
};

#endif
