#ifndef EVENT_MONITOR_H
#define EVENT_MONITOR_H

#include <QtGui>
#include <vector>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;
class Event;
struct EventMessage;

class EventMonitor : public QObject
{
    Q_OBJECT

    public:

        EventMonitor(MainWindow * mainWindow);

        boost::shared_ptr<EpidemicDataSet> getDataSet();
        int getTime();

        std::vector<boost::shared_ptr<EventMessage> > getMessages();

    signals:

        void clearMessages();
        void newEventMessage(boost::shared_ptr<EventMessage> message);

    public slots:

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        void setTime(int time);

        // check for new events on the latest time step
        void checkForEvents();

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;

        // events to monitor for
        std::vector<boost::shared_ptr<Event> > events_;

        // the resulting event messages
        std::vector<boost::shared_ptr<EventMessage> > messages_;
};

#endif
