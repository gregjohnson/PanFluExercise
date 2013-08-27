#include "TimelineWidget.h"
#include "EventMonitor.h"
#include "EventMessage.h"
#include "EpidemicDataSet.h"
#include "main.h"
#include "log.h"
#include "MainWindow.h"
#include <QSvgGenerator>

TimelineWidget::TimelineWidget(MainWindow* mainWindow, EventMonitor* monitor)
{
    monitor_ = monitor;

    QVBoxLayout * layout = new QVBoxLayout();
    setLayout(layout);

    //layout->addWidget(&messagesWidget_);

    // make connections
    connect(monitor, SIGNAL(clearMessages()), this, SLOT(clearMessages()));

    connect(monitor, SIGNAL(newEventMessage(boost::shared_ptr<EventMessage>)), this, SLOT(insertEventMessage(boost::shared_ptr<EventMessage>)));

    connect((QObject *)mainWindow, SIGNAL(numberOfTimestepsChanged()), this, SLOT(update()));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));

}

TimelineWidget::~TimelineWidget()
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

void TimelineWidget::paintEvent(QPaintEvent* event)
{
    makeCurrent();

    QPainter painter(this);

    render(&painter);
}

void TimelineWidget::render(QPainter* painter)
{
    painter->setBrush(QBrush(QColor::fromRgbF(1, 0, 0, 1)));
    painter->setPen(QPen(QBrush(QColor::fromRgbF(1, 0, 0, 1)), .1));

    int width = painter->window().width();
    int height = painter->window().height();

    const int totalShownTime = int(width / 30);

    int lastTime = 0;
    std::vector<boost::shared_ptr<EventMessage> > messages = monitor_->getMessages();

    //if (monitor_->messages_.size())
    //    lastTime = monitor_->messages_[monitor_->messages_.size()-1]->time;
    if (monitor_->getDataSet() && messages.size())
        lastTime = std::max<int>(monitor_->getDataSet()->getNumTimes()-1, messages[messages.size()-1]->time);

    const int currentSliderTime = std::max<int>(1, std::min<int>(time_, lastTime));

    const int firstShownTime = std::max<int>(currentSliderTime - totalShownTime, 1);
    const int lastShownTime = std::min<int>(firstShownTime + totalShownTime, lastTime);
    const int actualShownTime = lastShownTime - firstShownTime;

    //AARONBAD -- this is actually timeWindow+1. So if you want a window of 7, set to 6.
    const int timeWindow = 6;
    const float minTimeWindowRelativeSize = .5;

    int t = firstShownTime;

    float icon_wSpacing = .15;

    if ( (actualShownTime - timeWindow + 1) * icon_wSpacing * minTimeWindowRelativeSize > (timeWindow+1) * (1.f - minTimeWindowRelativeSize) )
    {
        //make icon_wSpacing whatever it needs to be
        icon_wSpacing = (timeWindow+1) * (1.f - minTimeWindowRelativeSize) / ((actualShownTime - timeWindow + 1) * minTimeWindowRelativeSize);
    }

    const float wSpacing = (width * 1.0) / ( (actualShownTime - timeWindow + 1) * icon_wSpacing + timeWindow+1 );

    //the minimum size of the timeWindow, relative to the full window size

    QFont titleFont = painter->font();
    titleFont.setPixelSize(std::min<float>(wSpacing / 10.f, 10.f));
    painter->setFont(titleFont);

    const float hSpacing = std::min<float>(10, height / 5);

    const int windowStartTime = std::max<int>(1, currentSliderTime - timeWindow);
    int h=1;
    float hOffset = hSpacing;
    float wOffset = wSpacing * icon_wSpacing;

    for(unsigned int i=0; i<messages.size(); i++)
    {
        boost::shared_ptr<EventMessage> message = messages[i];

        float wNextOffset = wOffset;

        if (message->time < firstShownTime || message->time > lastShownTime)
            continue;

        //draw axis ticks until we're caught up with current timestep
        while (t < message->time)
        {
            h = 1;
            char timeStr[4];
            sprintf(timeStr, "%d", t);

            if (t < windowStartTime || t > windowStartTime + timeWindow)
            {
                //not in time window
                painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));
                painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
                if (t != lastShownTime)
                {
                    if (t == firstShownTime && t > 1)
                        painter->drawRect(QRectF(wOffset - .1*wSpacing, hOffset + hSpacing * .5, wSpacing * .3, hSpacing * .1));
                    else
                        painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .5, wSpacing * .2, hSpacing * .1));
                }
                painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .45, wSpacing * .015, hSpacing * .2));
                if (actualShownTime <= 20 ||
                    (actualShownTime > 20 && actualShownTime <= 30 && (t-firstShownTime) % 2 == 0) ||
                    (actualShownTime > 30 && actualShownTime <= 40 && (t-firstShownTime) % 3 == 0) ||
                    (actualShownTime > 40 && actualShownTime <= 50 && (t-firstShownTime) % 4 == 0) ||
                    (actualShownTime > 50 && actualShownTime <= 100 && (t-firstShownTime) % 5 == 0) ||
                    (actualShownTime > 100 && actualShownTime <= 200 && (t-firstShownTime) % 10 == 0) ||
                    (actualShownTime > 200 && actualShownTime <= 500 && (t-firstShownTime) % 20 == 0) ||
                    (actualShownTime > 500 && (t-actualShownTime) % 50 == 0))
                        painter->drawText(QRectF(wOffset - wSpacing * .01, hOffset - hSpacing * .5, wSpacing * .2, hSpacing), QString(timeStr));
                wNextOffset = wOffset + wSpacing * icon_wSpacing;
            }
            else
            {
                //in time window
                painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));
                painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
                if (t != lastShownTime)
                {
                    if (t == firstShownTime && t > 1)
                        painter->drawRect(QRectF(wOffset - wSpacing * .1, hOffset + hSpacing * .5, wSpacing * 1.1, hSpacing * .1));
                    else
                        painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .5, wSpacing, hSpacing * .1));
                }
                painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .45, wSpacing * .05, hSpacing * .2));
                painter->drawText(QRectF(wOffset - wSpacing * .01, hOffset - hSpacing * .5, wSpacing * .2, hSpacing), QString(timeStr));
                wNextOffset = wOffset + wSpacing;
            }

            if (t < message->time)
                wOffset = wNextOffset;
            t++;
        }

        float iconSize = std::min<float>(wSpacing * icon_wSpacing * .9, hSpacing * .75);

        if (message->type == 0)
        {
            painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));
            painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
            painter->drawRect(QRectF(wOffset, hOffset + hSpacing * h, iconSize, iconSize));
        }
        else
        {
            painter->setBrush(QBrush(QColor::fromRgbF(1, 0, 0, 1)));
            painter->setPen(QPen(QBrush(QColor::fromRgbF(1, 0, 0, 1)), .1));
            painter->drawPie(QRectF(wOffset, hOffset + hSpacing * h, iconSize, iconSize), 0, 5760);
        }

        if (t >= windowStartTime && t <= windowStartTime + timeWindow)
        {
            painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));
            painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
            painter->drawText(QRectF(wOffset + wSpacing * .13, hOffset + hSpacing * h, wSpacing * .8, hSpacing * .9), QString(message->shortMessageText.c_str()));
        }

        wOffset = wNextOffset;
        h++;
    }

    //draw axis ticks until we're caught up with last timestep
    while (t <= lastShownTime)
    {
        h = 1;
        char timeStr[4];
        sprintf(timeStr, "%d", t);

        if (t < windowStartTime || t > windowStartTime + timeWindow)
        {
            painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));
            painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
            if (t != lastShownTime)
                painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .5, wSpacing * .2, hSpacing * .1));
            painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .45, wSpacing * .015, hSpacing * .2));
            if (actualShownTime <= 20 ||
                (actualShownTime > 20 && actualShownTime <= 30 && (t-firstShownTime) % 2 == 0) ||
                (actualShownTime > 30 && actualShownTime <= 40 && (t-firstShownTime) % 3 == 0) ||
                (actualShownTime > 40 && actualShownTime <= 50 && (t-firstShownTime) % 4 == 0) ||
                (actualShownTime > 50 && actualShownTime <= 100 && (t-firstShownTime) % 5 == 0) ||
                (actualShownTime > 100 && actualShownTime <= 200 && (t-firstShownTime) % 10 == 0) ||
                (actualShownTime > 200 && actualShownTime <= 500 && (t-firstShownTime) % 20 == 0) ||
                (actualShownTime > 500 && (t-actualShownTime) % 50 == 0))
                    painter->drawText(QRectF(wOffset - wSpacing * .01, hOffset - hSpacing * .5, wSpacing * .2, hSpacing), QString(timeStr));
            wOffset += wSpacing * icon_wSpacing;
        }
        else
        {
            painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));
            painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
            if (t != lastShownTime)
                painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .5, wSpacing, hSpacing * .1));
            else if (lastShownTime < lastTime)
                painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .5, wSpacing * .5, hSpacing * .1));

            painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .45, wSpacing * .05, hSpacing * .2));
            painter->drawText(QRectF(wOffset - wSpacing * .01, hOffset - hSpacing * .5, wSpacing * .2, hSpacing), QString(timeStr));
            wOffset += wSpacing;
        }

        t++;
    }
}

void TimelineWidget::exportSVGToDisplayCluster()
{
#if USE_DISPLAYCLUSTER
    if(g_dcSocket != NULL && svgTmpFile_.open())
    {
        QSvgGenerator generator;
        generator.setFileName(svgTmpFile_.fileName());
        generator.setResolution(90);
        QSize size(768, 70);
        QRect rect(QPoint(0,0), size);

        QPainter painter;
        painter.begin(&generator);

        painter.setWindow(rect);
        painter.setViewport(rect);

        // draw a white background
        painter.setBrush(QColor(255,255,255));
        painter.setPen(QColor(255,255,255));
        painter.drawRect(rect);

        render(&painter);

        painter.end();

        put_flog(LOG_DEBUG, "wrote %s", svgTmpFile_.fileName().toStdString().c_str());

        // now, send it to DisplayCluster
        sendSVGToDisplayCluster((svgTmpFile_.fileName()).toStdString(), (QString("ExerciseTimeline.svg")).toStdString());
    }
#endif
}

void TimelineWidget::setTime(int time)
{
    time_ = time;

    update();

    exportSVGToDisplayCluster();
}


void TimelineWidget::clearMessages()
{
    //messagesWidget_.clear();

    exportSVGToDisplayCluster();
}

void TimelineWidget::insertEventMessage(boost::shared_ptr<EventMessage> message)
{
    update();

    exportSVGToDisplayCluster();
}
