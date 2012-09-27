#include "EpidemicDataSet.h"
#include <netcdfcpp.h>
#include "log.h"

EpidemicDataSet::EpidemicDataSet(const char * filename)
{
    // defaults
    isValid_ = false;
    numTimes_ = 0;
    numNodes_ = 0;
    numStratifications_ = 0;

    if(loadNetCdfFile(filename) != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", filename);
        return;
    }
    else
    {
        isValid_ = true;
    }
}

bool EpidemicDataSet::isValid()
{
    return isValid_;
}

int EpidemicDataSet::getNumTimes()
{
    return numTimes_;
}

int EpidemicDataSet::getNumNodes()
{
    return numNodes_;
}

int EpidemicDataSet::getNumStratifications()
{
    return numStratifications_;
}

float EpidemicDataSet::getPopulation(int nodeId)
{
    if(nodeIdToIndex_.count(nodeId) == 0)
    {
        put_flog(LOG_ERROR, "could not map nodeId %i to an index", nodeId);
        return 0.;
    }

    return population_(nodeIdToIndex_[nodeId]);
}

std::vector<std::string> EpidemicDataSet::getVariableNames()
{
    std::vector<std::string> variableNames;

    std::map<std::string, blitz::Array<float, 3> >::iterator iter;

    for(iter=variables_.begin(); iter!=variables_.end(); iter++)
    {
        variableNames.push_back(iter->first);
    }

    return variableNames;
}

float EpidemicDataSet::getValue(std::string varName, int time, int nodeId)
{
    if(variables_.count(varName) == 0)
    {
        put_flog(LOG_ERROR, "no such variable %s", varName.c_str());
        return 0.;
    }
    else if(nodeId != NODES_ALL && nodeIdToIndex_.count(nodeId) == 0)
    {
        put_flog(LOG_ERROR, "could not map nodeId %i to an index", nodeId);
        return 0.;
    }

    // sum over all stratifications
    if(nodeId != NODES_ALL)
    {
        return blitz::sum(variables_[varName](time, nodeIdToIndex_[nodeId], blitz::Range::all()));
    }
    else
    {
        return blitz::sum(variables_[varName](time, blitz::Range::all(), blitz::Range::all()));
    }
}

bool EpidemicDataSet::loadNetCdfFile(const char * filename)
{
    // change netcdf library error behavior
    NcError err(NcError::verbose_nonfatal);

    // open the netcdf file
    NcFile ncFile(filename, NcFile::ReadOnly);

    if(!ncFile.is_valid())
    {
        put_flog(LOG_FATAL, "invalid file %s", filename);
        return false;
    }

    // get dimensions
    NcDim * timeDim = ncFile.get_dim("time");
    NcDim * nodesDim = ncFile.get_dim("nodes");
    NcDim * stratificationsDim = ncFile.get_dim("stratifications");

    if(timeDim == NULL || nodesDim == NULL || stratificationsDim == NULL)
    {
        put_flog(LOG_FATAL, "could not find a required dimension");
        return false;
    }

    numTimes_ = timeDim->size();
    numNodes_ = nodesDim->size();
    numStratifications_ = stratificationsDim->size();

    put_flog(LOG_DEBUG, "file contains %i timesteps, %i nodes, %i stratifications", numTimes_, numNodes_, numStratifications_);

    // get the required variables
    NcVar * ncVarIds = ncFile.get_var("ids_data");
    NcVar * ncVarPopulation = ncFile.get_var("population_data");
    NcVar * ncVarTravel = ncFile.get_var("travel_data");

    if(!ncVarIds->is_valid() || !ncVarPopulation->is_valid() || !ncVarTravel->is_valid())
    {
        put_flog(LOG_FATAL, "could not find a required variable");
        return false;
    }

    blitz::Array<int, 1> fipsIds((int *)ncVarIds->values()->base(), blitz::shape(numNodes_), blitz::duplicateData);
    blitz::Array<float, 1> population((float *)ncVarPopulation->values()->base(), blitz::shape(numNodes_), blitz::duplicateData);
    blitz::Array<float, 2> travel((float *)ncVarTravel->values()->base(), blitz::shape(numNodes_, numNodes_), blitz::duplicateData);

    // note the use of reference(). an = would make a copy, but we'd have to reshape the lhs first...
    fipsIds_.reference(fipsIds);
    population_.reference(population);
    travel_.reference(travel);

    // create node id to index mapping
    for(int i=0; i<numNodes_; i++)
    {
        nodeIdToIndex_[fipsIds_(i)] = i;
    }

    // get all float variables with dimensions (time, nodes, stratifications)
    for(int i=0; i<ncFile.num_vars(); i++)
    {
        NcVar * ncVar = ncFile.get_var(i);

        if(ncVar->num_dims() == 3 && ncVar->type() == ncFloat && strcmp(ncVar->get_dim(0)->name(), "time") == 0 && strcmp(ncVar->get_dim(1)->name(), "nodes") == 0 && strcmp(ncVar->get_dim(2)->name(), "stratifications") == 0)
        {
            put_flog(LOG_INFO, "found variable: %s", ncVar->name());

            blitz::Array<float, 3> var((float *)ncVar->values()->base(), blitz::shape(numTimes_, numNodes_, numStratifications_), blitz::duplicateData);

            variables_[std::string(ncVar->name())].reference(var);
        }
    }

    return true;
}
