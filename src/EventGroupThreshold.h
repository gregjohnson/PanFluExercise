#ifndef EVENT_GROUP_THRESHOLD_H
#define EVENT_GROUP_THRESHOLD_H

#include "Event.h"
#include <vector>
#include <string>

class EventMonitor;
struct EventMessage;

class EventGroupThreshold : public Event
{
    public:

        EventGroupThreshold(std::string groupName, std::string varName, std::vector<float> thresholds, bool fractional);

        boost::shared_ptr<EventMessage> check(EventMonitor * monitor);

    private:

        std::string groupName_;
        std::string varName_;
        std::vector<float> thresholds_;
        bool fractional_;

        // message corresponding to a detected event
        boost::shared_ptr<EventMessage> message_;
};

#endif
