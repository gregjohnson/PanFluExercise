#ifndef EVENT_H
#define EVENT_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>
#include <string>

class EventMonitor;
struct EventMessage;

class Event : public boost::enable_shared_from_this<Event>
{
    public:

        Event(std::vector<int> nodeIds, std::string varName, float threshold);

        boost::shared_ptr<EventMessage> check(EventMonitor * monitor);

    private:

        std::vector<int> nodeIds_;
        std::string varName_;
        float threshold_;

        // flag if event has already been detected
        bool detected_;

        // message corresponding to a detected event
        boost::shared_ptr<EventMessage> message_;
};

#endif
