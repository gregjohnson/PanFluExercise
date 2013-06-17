#include "EventMonitorWidget.h"
#include "EventMonitor.h"
#include "EventMessage.h"
#include "main.h"
#include "log.h"
#include <QSvgGenerator>

EventMonitorWidget::EventMonitorWidget(EventMonitor * monitor)
{
    QVBoxLayout * layout = new QVBoxLayout();
    setLayout(layout);

    layout->addWidget(&messagesWidget_);

    // make connections
    connect(monitor, SIGNAL(newEventMessage(boost::shared_ptr<EventMessage>)), this, SLOT(insertEventMessage(boost::shared_ptr<EventMessage>)));
}

EventMonitorWidget::~EventMonitorWidget()
{
    // the tempory file doesn't have the extension as written by the SVG exporter
    // so, we need to delete the file with the extension, too
    if(svgTmpFile_.fileName().isEmpty() != true)
    {
        QFile svgTmpFileOther(svgTmpFile_.fileName() + ".svg");

        if(svgTmpFileOther.exists() == true && svgTmpFileOther.remove() == true)
        {
            put_flog(LOG_DEBUG, "removed temporary file %s", svgTmpFileOther.fileName().toStdString().c_str());
        }
        else
        {
            put_flog(LOG_ERROR, "could not remove temporary file %s", svgTmpFileOther.fileName().toStdString().c_str());
        }
    }
}

void EventMonitorWidget::exportSVGToDisplayCluster()
{
    if(g_dcSocket != NULL && svgTmpFile_.open())
    {
        QSize size(1920, 1080);
        QRect rect(QPoint(0,0), size);

        QSvgGenerator svg;
        svg.setFileName(svgTmpFile_.fileName() + ".svg");
        svg.setSize(size);
        svg.setResolution(90);

        QTextDocument * doc = messagesWidget_.document()->clone();

        QPainter painter(&svg);

        // background
        painter.setBrush(QColor(200,200,200));
        painter.setPen(QColor(200,200,200));

        painter.drawRect(rect);

        doc->documentLayout()->setPaintDevice(&svg);

        painter.setViewport(rect);
        doc->setPageSize(size);

        doc->drawContents(&painter);

        painter.end();

        put_flog(LOG_DEBUG, "wrote %s", svgTmpFile_.fileName().toStdString().c_str());

        // now, send it to DisplayCluster
        sendSVGToDisplayCluster((svgTmpFile_.fileName()).toStdString() + ".svg", (QString("ExerciseEventMonitor.svg")).toStdString());
    }
}

void EventMonitorWidget::insertEventMessage(boost::shared_ptr<EventMessage> message)
{
    messagesWidget_.insertHtml(message->messageText.c_str() + QString("<br />"));

    exportSVGToDisplayCluster();
}
