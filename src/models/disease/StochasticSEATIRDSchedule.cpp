#include "StochasticSEATIRDSchedule.h"
#include "../../Parameters.h"
#include "../random.h"

StochasticSEATIRDSchedule::StochasticSEATIRDSchedule()
{
    Ta = 0.;
    Tt = 0.;
    Ti = 0.;
    Td_a = 0.;
    Td_ti = 0.;
    Tr_a = 0.;
    Tr_ti = 0.;
    Trd_ati = 0.;
}

StochasticSEATIRDSchedule::StochasticSEATIRDSchedule(const double now, MTRand &rand, std::vector<int> stratificationValues)
{
    Ta = now + random_exponential(g_parameters.getTau(), &rand);
    Tt =  Ta + random_exponential(g_parameters.getKappa(), &rand);
    Ti = Tt + g_parameters.getChi();
    Tr_a = Ta + random_exponential(g_parameters.getGamma(), &rand);
    Tr_ti = Tt + random_exponential(g_parameters.getGamma(), &rand);
    Td_a = Ta + random_exponential(g_parameters.getNu(), &rand);
    Td_ti = Tt + random_exponential(g_parameters.getNu(), &rand);

    // need to figure out whether the individual will recover or die, and whether from asymptomatic, treatable, or infectious

    double exitAsymptomaticTime = std::min(Td_a, Tr_a);

    // "exit time" here means via recovery or death (not progression to symptomatic stage)
    if(Tt < exitAsymptomaticTime)
    {
        exitAsymptomaticTime = std::numeric_limits<double>::infinity();
    }

    double exitInfectiousTime = std::min(Td_ti, Tr_ti);

    Trd_ati = std::min(exitAsymptomaticTime, exitInfectiousTime);
}
