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
		
	painter.begin(this);
	
	render(&painter);
		
	painter.end();
}

void TimelineWidget::render(QPainter* painter)
{	
    painter->setBrush(QBrush(QColor::fromRgbF(1, 0, 0, 1)));
    painter->setPen(QPen(QBrush(QColor::fromRgbF(1, 0, 0, 1)), .1));
	
	int width = painter->window().width();
	int height = painter->window().height();
	
	//idea: fix the window width; change the width of 
	
	int t = 1;
	int lastTime = 0;
	//if (monitor_->messages_.size())
	//	lastTime = monitor_->messages_[monitor_->messages_.size()-1]->time;
	if (monitor_->getDataSet() && monitor_->messages_.size())
		lastTime = std::max<int>(monitor_->getDataSet()->getNumTimes()-1, monitor_->messages_[monitor_->messages_.size()-1]->time);
	
	//AARONBAD -- this is actually timeWindow+1. So if you want a window of 7, set to 6.
	int timeWindow = 6;
	float wSpacing = (width * 1.0) / ( (lastTime - timeWindow + 1) * .15 + timeWindow+1 );	
	
    QFont titleFont = painter->font();
	titleFont.setPixelSize(wSpacing / 12.f);
    painter->setFont(titleFont);
	
		
	float hSpacing = std::min<float>(10, height / 5);
	
	int currentSliderTime = std::min<int>(time_, lastTime);
	//int currentSliderTime = lastTime;
			
	int windowStartTime = std::max<int>(1, currentSliderTime - timeWindow);
	int h=1;
	float hOffset = hSpacing;
	float wOffset = wSpacing * .15f;

	//draw the first timestep

    for(unsigned int i=0; i<monitor_->messages_.size(); i++)
    {
        boost::shared_ptr<EventMessage> message = monitor_->messages_[i];
			
		float wNextOffset = wOffset;
			
		//draw axis ticks until we're caught up with current timestep
		while (t < message->time)
		{
			h = 1;			
			char timeStr[4];
			sprintf(timeStr, "%d", t);

			if (t < windowStartTime || t > windowStartTime + timeWindow)
			{
			    painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));		
			    painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
				if (t != lastTime)
					painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .5, wSpacing * .2, hSpacing * .1));
				painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .45, wSpacing * .05, hSpacing * .2));
				if (lastTime <= 20 ||
					(lastTime > 20 && lastTime <= 30 && t % 2 == 0) ||
					(lastTime > 30 && lastTime <= 40 && t % 3 == 0) ||
					(lastTime > 40 && lastTime <= 50 && t % 4 == 0) ||
					(lastTime > 50 && lastTime <= 100 && t % 5 == 0) ||
					(lastTime > 100 && lastTime <= 200 && t % 10 == 0) ||		
					(lastTime > 200 && lastTime <= 500 && t % 20 == 0) ||	
					(lastTime > 500 && t % 50 == 0))
						painter->drawText(QRectF(wOffset - wSpacing * .01, hOffset - hSpacing * .5, wSpacing * .2, hSpacing), QString(timeStr));				
				wNextOffset = wOffset + wSpacing * .15;
			}
			else
			{
			    painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));		
			    painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
				if (t != lastTime)
					painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .5, wSpacing, hSpacing * .1));
				painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .45, wSpacing * .05, hSpacing * .2));
				painter->drawText(QRectF(wOffset - wSpacing * .01, hOffset - hSpacing * .5, wSpacing * .2, hSpacing), QString(timeStr));				
				wNextOffset = wOffset + wSpacing;
			}
			
			if (t < message->time)
				wOffset = wNextOffset;
			t++;
		}

		if (message->type == 0)
		{
	    	painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));		
	    	painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
			painter->drawRect(QRectF(wOffset, hOffset + hSpacing * h, hSpacing * 3/4, hSpacing * 3/4));
		}
		else
		{
	    	painter->setBrush(QBrush(QColor::fromRgbF(1, 0, 0, 1)));		
	    	painter->setPen(QPen(QBrush(QColor::fromRgbF(1, 0, 0, 1)), .1));
			painter->drawPie(QRectF(wOffset, hOffset + hSpacing * h, hSpacing * 3/4, hSpacing * 3/4), 0, 5760);		
		}
		
		if (t >= windowStartTime && t <= windowStartTime + timeWindow)
		{
		    painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));
		    painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
			painter->drawText(QRectF(wOffset + wSpacing * .12, hOffset + hSpacing * h, wSpacing * 4/5, hSpacing), QString(message->shortMessageText.c_str()));		
		}
				
		wOffset = wNextOffset;
		h++;	
    }

	//draw axis ticks until we're caught up with last timestep
	while (t <= lastTime)
	{
		h = 1;			
		char timeStr[4];
		sprintf(timeStr, "%d", t);

		if (t < windowStartTime || t > windowStartTime + timeWindow)
		{
		    painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));		
		    painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
			if (t != lastTime)
				painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .5, wSpacing * .2, hSpacing * .1));
			painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .45, wSpacing * .05, hSpacing * .2));		
			if (lastTime <= 20 ||
				(lastTime > 20 && lastTime <= 30 && t % 2 == 0) ||
				(lastTime > 30 && lastTime <= 40 && t % 3 == 0) ||
				(lastTime > 40 && lastTime <= 50 && t % 4 == 0) ||
				(lastTime > 50 && lastTime <= 100 && t % 5 == 0) ||
				(lastTime > 100 && lastTime <= 200 && t % 10 == 0) ||		
				(lastTime > 200 && lastTime <= 500 && t % 20 == 0) ||	
				(lastTime > 500 && t % 50 == 0))
					painter->drawText(QRectF(wOffset - wSpacing * .01, hOffset - hSpacing * .5, wSpacing * .2, hSpacing), QString(timeStr));
			wOffset += wSpacing * .15;
		}
		else
		{
		    painter->setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 1)));		
		    painter->setPen(QPen(QBrush(QColor::fromRgbF(0, 0, 0, 1)), .1));
			if (t != lastTime)
				painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .5, wSpacing, hSpacing * .1));
			painter->drawRect(QRectF(wOffset, hOffset + hSpacing * .45, wSpacing * .05, hSpacing * .2));	
			painter->drawText(QRectF(wOffset - wSpacing * .01, hOffset - hSpacing * .5, wSpacing * .2, hSpacing), QString(timeStr));
			wOffset += wSpacing;
		}

		t++;
	}

	
}

void TimelineWidget::exportSVGToDisplayCluster()
{
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
