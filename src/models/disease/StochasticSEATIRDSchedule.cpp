#include "StochasticSEATIRDSchedule.h"
#include "../../Parameters.h"
#include "../random.h"
#include "../../log.h"

StochasticSEATIRDSchedule::StochasticSEATIRDSchedule(const double &now, MTRand &rand, const std::vector<int> &stratificationValues)
{
    stratificationValues_ = stratificationValues;

    // the individual starts as exposed
    state_ = E;

    // the schedule can later be canceled, but starts out active
    canceled_ = false;

    // generate all transitions starting from "exposed"

    // time to progress from exposed to asymptomatic
    double Ta = now + random_exponential(1. / g_parameters.getTau(), &rand);

    // infected period begins at asymptomatic
    infectedTMin_ = Ta;

    // infected periods ends at recovery / death and is set inline below

    eventQueue_.push(StochasticSEATIRDEvent(now, Ta, EtoA, stratificationValues, stratificationValues));

    // compute nu (rate) from nu (CFR)
    double nu = -1./g_parameters.getGamma() * log(1. - g_parameters.getNu(stratificationValues[0]));

    // asymptomatic transition: -> treatable, -> recovered, or -> deceased
    double Tt =  Ta + random_exponential(1. / g_parameters.getKappa(), &rand); // time to progress from asymptomatic to treatable
    double Tr_a = Ta + random_exponential(1. / g_parameters.getGamma(), &rand); // time to recover from asymptomatic
    double Td_a = Ta + random_exponential(nu, &rand); // time to death from asymptomatic

    if(Tt < Tr_a && Tt < Td_a)
    {
        // -> treatable
        eventQueue_.push(StochasticSEATIRDEvent(Ta, Tt, AtoT, stratificationValues, stratificationValues));

        // treatable transitions: -> infectious, -> recovered, or -> deceased
        double Ti = Tt + g_parameters.getChi(); // time to progress from treatable to infectious
        double Tr_ti = Tt + random_exponential(1. / g_parameters.getGamma(), &rand); // time to recover from treatable/infectious
        double Td_ti = Tt + random_exponential(nu, &rand); // time to death from treatable/infectious

        if(Ti < Tr_ti && Ti < Td_ti)
        {
            // -> infectious
            eventQueue_.push(StochasticSEATIRDEvent(Tt, Ti, TtoI, stratificationValues, stratificationValues));

            // infectious transitions: -> recovered, or -> deceased
            if(Tr_ti < Td_ti)
            {
                // -> recovered
                infectedTMax_ = Tr_ti;
                eventQueue_.push(StochasticSEATIRDEvent(Ti, Tr_ti, ItoR, stratificationValues, stratificationValues));
            }
            else // Td_ti < Tr_ti
            {
                // -> deceased
                infectedTMax_ = Td_ti;
                eventQueue_.push(StochasticSEATIRDEvent(Ti, Td_ti, ItoD, stratificationValues, stratificationValues));
            }
        }
        else if(Tr_ti < Td_ti)
        {
            // -> recovered
            infectedTMax_ = Tr_ti;
            eventQueue_.push(StochasticSEATIRDEvent(Tt, Tr_ti, TtoR, stratificationValues, stratificationValues));
        }
        else // Td_ti < Tr_ti
        {
            // -> deceased
            infectedTMax_ = Td_ti;
            eventQueue_.push(StochasticSEATIRDEvent(Tt, Td_ti, TtoD, stratificationValues, stratificationValues));
        }
    }
    else if(Tr_a < Td_a)
    {
        // -> recovered
        infectedTMax_ = Tr_a;
        eventQueue_.push(StochasticSEATIRDEvent(Ta, Tr_a, AtoR, stratificationValues, stratificationValues));
    }
    else // Td_a < Tr_a
    {
        // -> deceased
        infectedTMax_ = Td_a;
        eventQueue_.push(StochasticSEATIRDEvent(Ta, Td_a, AtoD, stratificationValues, stratificationValues));
    }
}

void StochasticSEATIRDSchedule::insertEvent(const StochasticSEATIRDEvent &event)
{
    eventQueue_.push(event);
}

bool StochasticSEATIRDSchedule::empty() const
{
    return eventQueue_.empty();
}

StochasticSEATIRDEvent StochasticSEATIRDSchedule::getTopEvent() const
{
    return eventQueue_.top();
}

void StochasticSEATIRDSchedule::popTopEvent()
{
    if(canceled_ == true)
    {
        put_flog(LOG_WARN, "schedule is canceled");
    }

    // when the event is popped, it is assumed it is processed and the individual transitions states accordingly
    StochasticSEATIRDEventType type = eventQueue_.top().type;

    // this should handle all event types involving transitions
    switch(type)
    {
        case EtoA:
            state_ = A;
            break;

        case AtoT:
            state_ = T;
            break;
        case AtoR:
            state_ = R;
            break;
        case AtoD:
            state_ = D;
            break;

        case TtoI:
            state_ = I;
            break;
        case TtoR:
            state_ = R;
            break;
        case TtoD:
            state_ = D;
            break;

        case ItoR:
            state_ = R;
            break;
        case ItoD:
            state_ = D;
            break;
    }

    eventQueue_.pop();
}

std::vector<int> StochasticSEATIRDSchedule::getStratificationValues() const
{
    return stratificationValues_;
}

StochasticSEATIRDScheduleState StochasticSEATIRDSchedule::getState() const
{
    return state_;
}

double StochasticSEATIRDSchedule::getInfectedTMin() const
{
    return infectedTMin_;
}

double StochasticSEATIRDSchedule::getInfectedTMax() const
{
    return infectedTMax_;
}

bool StochasticSEATIRDSchedule::canceled() const
{
    return canceled_;
}

void StochasticSEATIRDSchedule::cancel()
{
    canceled_ = true;
}

void StochasticSEATIRDSchedule::changeStratificationValues(std::vector<int> stratificationValues)
{
    stratificationValues_ = stratificationValues;

    // update fromValues in events in queue
    boost::heap::pairing_heap<StochasticSEATIRDEvent, boost::heap::compare<StochasticSEATIRDEvent::compareByTime> >::iterator begin = eventQueue_.begin();
    boost::heap::pairing_heap<StochasticSEATIRDEvent, boost::heap::compare<StochasticSEATIRDEvent::compareByTime> >::iterator end = eventQueue_.end();

    boost::heap::pairing_heap<StochasticSEATIRDEvent, boost::heap::compare<StochasticSEATIRDEvent::compareByTime> >::iterator it;

    for(it=begin; it!=end; it++)
    {
        (*boost::heap::pairing_heap<StochasticSEATIRDEvent, boost::heap::compare<StochasticSEATIRDEvent::compareByTime> >::s_handle_from_iterator(it)).fromStratificationValues = stratificationValues;
    }
}
