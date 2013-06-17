#include "Event.h"
#include "EventMonitor.h"
#include "EventMessage.h"
#include "EpidemicDataSet.h"
#include "log.h"

Event::Event(std::vector<int> nodeIds, std::string varName, float threshold)
{
    // defaults
    detected_ = false;

    // assign values
    nodeIds_ = nodeIds;
    varName_ = varName;
    threshold_ = threshold;
}

boost::shared_ptr<EventMessage> Event::check(EventMonitor * monitor)
{
    // return existing message if we already detected the event, or see if the event has occurred and return the appropriate message
    // otherwise, return NULL
    if(detected_ == true)
    {
        return message_;
    }
    else
    {
        // get value and see if we've exceeded the threshold

        // the dataset
        boost::shared_ptr<EpidemicDataSet> dataSet = monitor->getDataSet();

        if(dataSet == NULL)
        {
            put_flog(LOG_ERROR, "unable to get EpidemicDataSet");
            return boost::shared_ptr<EventMessage>();
        }

        float value = 0.;

        for(unsigned int i=0; i<nodeIds_.size(); i++)
        {
            value += dataSet->getValue(varName_, dataSet->getNumTimes()-1, nodeIds_[i]);
        }

        if(value >= threshold_)
        {
            detected_ = true;

            std::string messageString = "<b>" + varName_ + "</b> exceeded the threshold.";

            message_ = boost::shared_ptr<EventMessage>(new EventMessage(shared_from_this(), messageString));

            return message_;
        }
    }

    return boost::shared_ptr<EventMessage>();
}
