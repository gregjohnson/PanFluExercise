#include "EpidemicDataSet.h"
#include "config.h"
#include "log.h"
#include <netcdfcpp.h>
#include <fstream>
#include <boost/tokenizer.hpp>

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

        // read node name data
        std::string nodeNameFilename = std::string(ROOT_DIRECTORY) + "/data/fips_county_names_HSRs.csv";
        std::ifstream in(nodeNameFilename.c_str());

        if(in.is_open() != true)
        {
            put_flog(LOG_ERROR, "could not load file %s", nodeNameFilename.c_str());
            return;
        }

        // use boost tokenizer to parse the file
        typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

        std::vector<std::string> vec;
        std::string line;

        while(getline(in, line))
        {
            Tokenizer tok(line);

            vec.assign(tok.begin(), tok.end());

            if(vec.size() != 3)
            {
                put_flog(LOG_ERROR, "number of values != 3, == %i", vec.size());
                return;
            }

            // nodeId -> name mapping
            nodeIdToName_[atoi(vec[0].c_str())] = vec[1];

            // nodeId -> group name mapping
            nodeIdToGroupName_[atoi(vec[0].c_str())] = vec[2];

            // group name -> nodeIds indexing
            groupNameToNodeIds_[vec[2]].push_back(atoi(vec[0].c_str()));
        }
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

std::string EpidemicDataSet::getNodeName(int nodeId)
{
    if(nodeIdToName_.count(nodeId) == 0)
    {
        put_flog(LOG_ERROR, "could not map nodeId %i to a name", nodeId);
        return std::string("");
    }

    return nodeIdToName_[nodeId];
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

std::vector<int> EpidemicDataSet::getNodeIds()
{
    std::vector<int> nodeIds(nodeIds_.begin(), nodeIds_.end());

    return nodeIds;
}

std::vector<std::string> EpidemicDataSet::getGroupNames()
{
    std::vector<std::string> groupNames;

    for(std::map<std::string, std::vector<int> >::iterator it=groupNameToNodeIds_.begin(); it!=groupNameToNodeIds_.end(); it++)
    {
        groupNames.push_back(it->first);
    }

    return groupNames;
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

float EpidemicDataSet::getValue(std::string varName, int time, std::string groupName)
{
    if(variables_.count(varName) == 0)
    {
        put_flog(LOG_ERROR, "no such variable %s", varName.c_str());
        return 0.;
    }
    else if(groupNameToNodeIds_.count(groupName) == 0)
    {
        put_flog(LOG_ERROR, "could not map group name %s to node ids", groupName.c_str());
        return 0.;
    }

    std::vector<int> nodeIds = groupNameToNodeIds_[groupName];

    float value = 0.;

    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        value += getValue(varName, time, nodeIds[i]);
    }

    return value;
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

    blitz::Array<int, 1> nodeIds((int *)ncVarIds->values()->base(), blitz::shape(numNodes_), blitz::duplicateData);
    blitz::Array<float, 1> population((float *)ncVarPopulation->values()->base(), blitz::shape(numNodes_), blitz::duplicateData);
    blitz::Array<float, 2> travel((float *)ncVarTravel->values()->base(), blitz::shape(numNodes_, numNodes_), blitz::duplicateData);

    // note the use of reference(). an = would make a copy, but we'd have to reshape the lhs first...
    nodeIds_.reference(nodeIds);
    population_.reference(population);
    travel_.reference(travel);

    // create node id to index mapping
    for(int i=0; i<numNodes_; i++)
    {
        nodeIdToIndex_[nodeIds_(i)] = i;
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
