#ifndef EVENT_MESSAGE_H
#define EVENT_MESSAGE_H

#include <boost/weak_ptr.hpp>
#include <string>

class Event;

struct EventMessage
{
    EventMessage(boost::shared_ptr<Event> e, std::string m)
    {
        event = e;
        messageText = m;
    }

    // weak pointer to prevent cyclic references
    boost::weak_ptr<Event> event;

    std::string messageText;
};

#endif
