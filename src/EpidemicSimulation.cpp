#include "EpidemicSimulation.h"
#include "StockpileNetwork.h"
#include "log.h"

EpidemicSimulation::EpidemicSimulation()
{
    put_flog(LOG_DEBUG, "");

    // EpidemicDataSet gives us only the population variable

    // the only generic required variables are "susceptible" and "exposed"

    // copy full population to susceptible
    copyVariable("population", "susceptible");

    // an empty exposed variable
    newVariable("exposed");

    // create basic StockpileNetwork
    boost::shared_ptr<StockpileNetwork> stockpileNetwork(new StockpileNetwork());

    // add stockpiles for each group
    std::vector<std::string> groupNames = getGroupNames();

    for(unsigned int i=0; i<groupNames.size(); i++)
    {
        boost::shared_ptr<Stockpile> stockpile(new Stockpile(groupNames[i]));

        std::vector<int> nodeIds = getNodeIds(groupNames[i]);
        stockpile->setNodeIds(nodeIds);

        // todo: for now a default stockpile inventory
        stockpile->setNum(100000);

        stockpileNetwork->addStockpile(stockpile);
    }

    stockpileNetwork_ = stockpileNetwork;
}

int EpidemicSimulation::expose(int num, int nodeId, std::vector<int> stratificationValues)
{
    return transition(num, "susceptible", "exposed", nodeId, stratificationValues);
}


void EpidemicSimulation::simulate()
{
    put_flog(LOG_DEBUG, "");

    // copy all variables to a new time
    std::map<std::string, blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> >::iterator iter;

    for(iter=variables_.begin(); iter!=variables_.end(); iter++)
    {
        copyVariableToNewTimeStep(iter->first);
    }

    numTimes_++;
}

int EpidemicSimulation::transition(int num, std::string sourceVarName, std::string destVarName, int nodeId, std::vector<int> stratificationValues)
{
    blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> sourceVarAtFinalTime = getVariableAtFinalTime(sourceVarName);
    blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> destVarAtFinalTime = getVariableAtFinalTime(destVarName);

    if(sourceVarAtFinalTime.size() == 0 || destVarAtFinalTime.size() == 0)
    {
        put_flog(LOG_ERROR, "could not transition, one of the variables does not exist");
        return 0.;
    }

    // todo: validate nodeIndex, stratification values are in bounds

    int numSourceVar = sourceVarAtFinalTime(nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues));

    int numTransition = num;

    if(numTransition > numSourceVar)
    {
        numTransition = numSourceVar;
    }

    sourceVarAtFinalTime(nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues)) -= numTransition;

    destVarAtFinalTime(nodeIdToIndex_[nodeId], BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, VECTOR_TO_ARGS, stratificationValues)) += numTransition;

    return numTransition;
}
