#include "EventGroupThreshold.h"
#include "EventMonitor.h"
#include "EventMessage.h"
#include "EpidemicDataSet.h"
#include "log.h"
#include <boost/lexical_cast.hpp>

EventGroupThreshold::EventGroupThreshold(std::string groupName, std::string varName, std::vector<float> thresholds, bool fractional)
{
    // assign values
    groupName_ = groupName;
    varName_ = varName;
    thresholds_ = thresholds;
    fractional_ = fractional;
}

boost::shared_ptr<EventMessage> EventGroupThreshold::check(EventMonitor * monitor)
{
    // see if the event has occurred and return the appropriate message
    // otherwise, return NULL
    // when a threshold event occurs, the lower thresholds will also be erased

    // the dataset
    boost::shared_ptr<EpidemicDataSet> dataSet = monitor->getDataSet();

    if(dataSet == NULL)
    {
        put_flog(LOG_ERROR, "unable to get EpidemicDataSet");
        return boost::shared_ptr<EventMessage>();
    }

    int time = dataSet->getNumTimes()-1;

    for(int i=thresholds_.size()-1; i>=0; i--)
    {
        float value = dataSet->getValue(varName_, dataSet->getNumTimes()-1, groupName_);
        float population = dataSet->getValue("population", dataSet->getNumTimes()-1, groupName_);

        if(fractional_ == false && value >= thresholds_[i])
        {
            std::string messageString = "<b>Day " + boost::lexical_cast<std::string>(time) + "</b>: ";
            std::string shortMessageString = "";

            if(varName_ == "deceased")
            {
                messageString += "There are now " + boost::lexical_cast<std::string>((int)value) + " deceased individuals in " + groupName_ + ".";
                shortMessageString += boost::lexical_cast<std::string>((int)value) +  "d-" + groupName_ + "";
            }
            else
            {
                messageString = "cannot generate event message for variable " + varName_;
            }

            message_ = boost::shared_ptr<EventMessage>(new EventMessage(shared_from_this(), messageString, shortMessageString, time, 0));

            // erase this and previous thresholds, since they've already been exceeded
            thresholds_.erase(thresholds_.begin(), thresholds_.begin() + i+1);

            return message_;
        }
        else if(fractional_ == true && value >= thresholds_[i] * population)
        {
            // round to 10th of a percent
            char percentageString[64];
            sprintf(percentageString, "%.2f", value / population * 100.);

            std::string messageString = "<b>Day " + boost::lexical_cast<std::string>(time) + "</b>: ";
            std::string shortMessageString = "";

            if(varName_ == "All infected")
            {
                messageString += boost::lexical_cast<std::string>(percentageString) + "% (" +  boost::lexical_cast<std::string>((int)value) + " individuals) are now infected in " + groupName_ + ".";
                shortMessageString += boost::lexical_cast<std::string>(percentageString) + "%i-" + groupName_ + "";
            }
            else
            {
                messageString = "cannot generate event message for variable " + varName_;
            }

            // erase this and previous thresholds, since they've already been exceeded
            thresholds_.erase(thresholds_.begin(), thresholds_.begin() + i+1);

            message_ = boost::shared_ptr<EventMessage>(new EventMessage(shared_from_this(), messageString, shortMessageString, time, 1));

            return message_;
        }
    }

    return boost::shared_ptr<EventMessage>();
}
