#include "EventMonitor.h"
#include "MainWindow.h"
#include "EpidemicDataSet.h"
#include "Event.h"
#include "EventGroupThreshold.h"
#include "EventMessage.h"
#include "log.h"

EventMonitor::EventMonitor(MainWindow * mainWindow)
{
    // set ownership
    setParent(mainWindow);

    // defaults
    time_ = 0;

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect((QObject *)mainWindow, SIGNAL(numberOfTimestepsChanged()), this, SLOT(checkForEvents()));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));
}

boost::shared_ptr<EpidemicDataSet> EventMonitor::getDataSet()
{
    return dataSet_;
}

int EventMonitor::getTime()
{
    return time_;
}

std::vector<boost::shared_ptr<EventMessage> > EventMonitor::getMessages()
{
    return messages_;
}

void EventMonitor::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    // clear existing events and messages
    events_.clear();
    messages_.clear();

    emit(clearMessages());

    if(dataSet == NULL)
    {
        return;
    }

    // new default events
    // todo: change these to something for sensible...
    std::vector<std::string> groupNames = dataSet->getGroupNames();

    for(unsigned int i=0; i<groupNames.size(); i++)
    {
        std::vector<float> infectedThresholds;
        infectedThresholds.push_back(0.0025);
        infectedThresholds.push_back(0.005);
        infectedThresholds.push_back(0.0075);
        infectedThresholds.push_back(0.01);

        boost::shared_ptr<Event> event1(new EventGroupThreshold(groupNames[i], "All infected", infectedThresholds, true));

        std::vector<float> deceasedThresholds;
        deceasedThresholds.push_back(1);
        deceasedThresholds.push_back(5);
        deceasedThresholds.push_back(10);
        deceasedThresholds.push_back(25);
        deceasedThresholds.push_back(50);
        deceasedThresholds.push_back(100);
        deceasedThresholds.push_back(250);
        deceasedThresholds.push_back(500);
        deceasedThresholds.push_back(1000);

        boost::shared_ptr<Event> event2(new EventGroupThreshold(groupNames[i], "deceased", deceasedThresholds, false));

        events_.push_back(event1);
        events_.push_back(event2);
    }
}

void EventMonitor::setTime(int time)
{
    time_ = time;
}

void EventMonitor::checkForEvents()
{
    for(unsigned int i=0; i<events_.size(); i++)
    {
        boost::shared_ptr<EventMessage> message = events_[i]->check(this);

        if(message != NULL)
        {
            put_flog(LOG_DEBUG, "detected event: %s", message->messageText.c_str());

            messages_.push_back(message);

            emit(newEventMessage(message));
        }
    }
}
