#include "EpidemicDataSet.h"
#include "main.h"
#include "log.h"
#include <netcdfcpp.h>
#include <fstream>
#include <boost/tokenizer.hpp>

std::vector<std::string> EpidemicDataSet::stratificationNames_;
std::vector<std::vector<std::string> > EpidemicDataSet::stratifications_;

EpidemicDataSet::EpidemicDataSet(const char * filename)
{
    // defaults
    isValid_ = false;
    numTimes_ = 0;
    numNodes_ = 0;
    numStratifications_ = 0;

    // load node name and group data first
    std::string nodeNameGroupFilename = g_dataDirectory + "/fips_county_names_HSRs.csv";

    if(loadNodeNameGroupFile(nodeNameGroupFilename.c_str()) != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", nodeNameGroupFilename.c_str());
        return;
    }

    if(loadNetCdfFile(filename) != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", filename);
        return;
    }

    isValid_ = true;
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

std::vector<std::string> EpidemicDataSet::getStratificationNames()
{
    // one time only: initialize stratification data
    if(stratificationNames_.size() == 0)
    {
        stratificationNames_.push_back("age group");
        stratificationNames_.push_back("risk group");
        stratificationNames_.push_back("vaccinated");
    }

    return stratificationNames_;
}

std::vector<std::vector<std::string> > EpidemicDataSet::getStratifications()
{
    // one time only: initialize stratification data
    if(stratifications_.size() == 0)
    {
        std::vector<std::string> ageGroups;
        ageGroups.push_back("0-4 years");
        ageGroups.push_back("5-24 years");
        ageGroups.push_back("25-49 years");
        ageGroups.push_back("50-64 years");
        ageGroups.push_back("65+ years");

        std::vector<std::string> riskGroups;
        riskGroups.push_back("low risk");
        riskGroups.push_back("high risk");

        std::vector<std::string> vaccinationGroups;
        vaccinationGroups.push_back("unvaccinated");
        vaccinationGroups.push_back("vaccinated");

        stratifications_.push_back(ageGroups);
        stratifications_.push_back(riskGroups);
        stratifications_.push_back(vaccinationGroups);
    }

    return stratifications_;
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

    std::map<std::string, blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> >::iterator iter;

    for(iter=variables_.begin(); iter!=variables_.end(); iter++)
    {
        variableNames.push_back(iter->first);
    }

    return variableNames;
}

std::vector<int> EpidemicDataSet::getNodeIds()
{
    return nodeIds_;
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

float EpidemicDataSet::getValue(std::string varName, int time, int nodeId, std::vector<int> stratificationValues)
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

    // the full domain
    blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> lowerBound = variables_[varName].lbound();
    blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> upperBound = variables_[varName].ubound();

    // limit by time
    lowerBound(0) = upperBound(0) = time;

    // limit by node
    if(nodeId != NODES_ALL)
    {
        lowerBound(1) = upperBound(1) = nodeIdToIndex_[nodeId];
    }

    // limit by stratification values
    for(unsigned int i=0; i<stratificationValues.size(); i++)
    {
        if(stratificationValues[i] != STRATIFICATIONS_ALL)
        {
            lowerBound(2+i) = upperBound(2+i) = stratificationValues[i];
        }
    }

    // the subdomain
    blitz::RectDomain<2+NUM_STRATIFICATION_DIMENSIONS> subdomain(lowerBound, upperBound);

    // form an array over the domain
    blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> varSubdomain = variables_[varName](subdomain);

    // and return the sum
    return blitz::sum(varSubdomain);
}

float EpidemicDataSet::getValue(std::string varName, int time, std::string groupName, std::vector<int> stratificationValues)
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
        value += getValue(varName, time, nodeIds[i], stratificationValues);
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

    // make sure number of nodes matches our expectation...
    if(numNodes_ != (int)nodeIds_.size())
    {
        put_flog(LOG_FATAL, "got %i nodes, expected %i", numNodes_, nodeIds_.size());
        return false;
    }

    // make sure number of stratifications matches our expectation...
    int numExpectedStratifications = 1;

    for(unsigned int i=0; i<NUM_STRATIFICATION_DIMENSIONS; i++)
    {
        numExpectedStratifications *= stratifications_[i].size();
    }

    if(numStratifications_ != numExpectedStratifications)
    {
        put_flog(LOG_FATAL, "got %i stratifications, expected %i", numStratifications_, numExpectedStratifications);
        return false;
    }

    // get the required variables
    NcVar * ncVarPopulation = ncFile.get_var("population_data");
    NcVar * ncVarTravel = ncFile.get_var("travel_data");

    if(!ncVarPopulation->is_valid() || !ncVarTravel->is_valid())
    {
        put_flog(LOG_FATAL, "could not find a required variable");
        return false;
    }

    blitz::Array<float, 1> population((float *)ncVarPopulation->values()->base(), blitz::shape(numNodes_), blitz::duplicateData);
    blitz::Array<float, 2> travel((float *)ncVarTravel->values()->base(), blitz::shape(numNodes_, numNodes_), blitz::duplicateData);

    // note the use of reference(). an = would make a copy, but we'd have to reshape the lhs first...
    population_.reference(population);
    travel_.reference(travel);

    // get all float variables with dimensions (time, nodes, stratifications)
    for(int i=0; i<ncFile.num_vars(); i++)
    {
        NcVar * ncVar = ncFile.get_var(i);

        if(ncVar->num_dims() == 3 && ncVar->type() == ncFloat && strcmp(ncVar->get_dim(0)->name(), "time") == 0 && strcmp(ncVar->get_dim(1)->name(), "nodes") == 0 && strcmp(ncVar->get_dim(2)->name(), "stratifications") == 0)
        {
            put_flog(LOG_INFO, "found variable: %s", ncVar->name());

            // full shape
            blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> shape;
            shape(0) = numTimes_;
            shape(1) = numNodes_;

            for(int j=0; j<NUM_STRATIFICATION_DIMENSIONS; j++)
            {
                shape(2 + j) = stratifications_[j].size();
            }

            blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> var((float *)ncVar->values()->base(), shape, blitz::duplicateData);

            variables_[std::string(ncVar->name())].reference(var);
        }
    }

    return true;
}

bool EpidemicDataSet::loadNodeNameGroupFile(const char * filename)
{
    std::ifstream in(filename);

    if(in.is_open() != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", filename);
        return false;
    }

    // clear existing entries
    nodeIds_.clear();
    nodeIdToIndex_.clear();
    nodeIdToName_.clear();
    nodeIdToGroupName_.clear();
    groupNameToNodeIds_.clear();

    // use boost tokenizer to parse the file
    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

    std::vector<std::string> vec;
    std::string line;

    int index = 0;

    while(getline(in, line))
    {
        Tokenizer tok(line);

        vec.assign(tok.begin(), tok.end());

        if(vec.size() != 3)
        {
            put_flog(LOG_ERROR, "number of values != 3, == %i", vec.size());
            return false;
        }

        // nodeId vector
        nodeIds_.push_back(atoi(vec[0].c_str()));

        // nodeId -> index mapping
        nodeIdToIndex_[atoi(vec[0].c_str())] = index;

        // nodeId -> name mapping
        nodeIdToName_[atoi(vec[0].c_str())] = vec[1];

        // nodeId -> group name mapping
        nodeIdToGroupName_[atoi(vec[0].c_str())] = vec[2];

        // group name -> nodeIds indexing
        groupNameToNodeIds_[vec[2]].push_back(atoi(vec[0].c_str()));

        index++;
    }

    return true;
}
