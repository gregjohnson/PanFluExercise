#ifndef EVENT_H
#define EVENT_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class EventMonitor;
struct EventMessage;

class Event : public boost::enable_shared_from_this<Event>
{
    public:

        Event();

        virtual boost::shared_ptr<EventMessage> check(EventMonitor * monitor) = 0;
};

#endif
