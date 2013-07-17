#ifndef EVENT_MESSAGE_H
#define EVENT_MESSAGE_H

#include <boost/weak_ptr.hpp>
#include <string>

class Event;

struct EventMessage
{
    EventMessage(boost::shared_ptr<Event> e, std::string m, std::string sm, int _time = 0, int _type = 0)
    {
        event = e;
        messageText = m;
		shortMessageText = sm;
		time = _time;
		type = _type;
    }

    // weak pointer to prevent cyclic references
    boost::weak_ptr<Event> event;

    std::string messageText;
	std::string shortMessageText;
	int time;
	int type;
};

#endif
