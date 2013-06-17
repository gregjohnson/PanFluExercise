#ifndef EVENT_MONITOR_WIDGET_H
#define EVENT_MONITOR_WIDGET_H

#include <QtGui>
#include <boost/shared_ptr.hpp>

class EventMonitor;
struct EventMessage;

class EventMonitorWidget : public QWidget
{
    Q_OBJECT

    public:

        EventMonitorWidget(EventMonitor * monitor);
        ~EventMonitorWidget();

        void exportSVGToDisplayCluster();

    public slots:

        void clearMessages();
        void insertEventMessage(boost::shared_ptr<EventMessage> message);

    private:

        QTextEdit messagesWidget_;

        // SVG export
        QTemporaryFile svgTmpFile_;
};

#endif
