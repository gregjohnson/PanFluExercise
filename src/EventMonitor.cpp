#include "EventMonitor.h"
#include "MainWindow.h"
#include "EpidemicDataSet.h"
#include "Event.h"
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

void EventMonitor::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    // clear existing events and messages
    events_.clear();
    messages_.clear();

    if(dataSet == NULL)
    {
        return;
    }

    // new default events
    // todo: change these to something for sensible...
    std::vector<std::string> groupNames = dataSet->getGroupNames();

    for(unsigned int i=0; i<groupNames.size(); i++)
    {
        std::vector<int> nodeIds = dataSet->getNodeIds(groupNames[i]);

        boost::shared_ptr<Event> event(new Event(nodeIds, ":infected", 5000.));

        events_.push_back(event);
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
