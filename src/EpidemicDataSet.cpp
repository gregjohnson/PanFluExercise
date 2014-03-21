#include "EpidemicDataSet.h"
#include "main.h"
#include "log.h"
#include <fstream>
#include <boost/tokenizer.hpp>

#if USE_NETCDF
    #include <netcdfcpp.h>
#endif

std::vector<std::string> EpidemicDataSet::stratificationNames_;
std::vector<std::vector<std::string> > EpidemicDataSet::stratifications_;

EpidemicDataSet::EpidemicDataSet(const char * filename)
{
    // defaults
    isValid_ = false;
    numTimes_ = 1;
    numNodes_ = 0;

    // load stratifications data
    if(loadStratificationsFile() != true)
    {
        put_flog(LOG_ERROR, "could not load stratifications file");
        return;
    }

    // load node name and group data
    std::string nodeNameGroupFilename = g_dataDirectory + "/fips_county_names_HSRs.csv";

    if(loadNodeNameGroupFile(nodeNameGroupFilename.c_str()) != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", nodeNameGroupFilename.c_str());
        return;
    }

    // population data
    std::string nodePopulationFilename = g_dataDirectory + "/fips_age_group_populations.csv";

    if(loadNodePopulationFile(nodePopulationFilename.c_str()) != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", nodePopulationFilename.c_str());
        return;
    }

    std::string nodePopulationSecondStratificationFilename = g_dataDirectory + "/age_groups_low_risk_fraction.csv";

    if(loadNodePopulationSecondStratificationFile(nodePopulationSecondStratificationFilename.c_str()) != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", nodePopulationSecondStratificationFilename.c_str());
        return;
    }

    // travel data
    std::string nodeTravelFilename = g_dataDirectory + "/county_travel_fractions.csv";

    if(loadNodeTravelFile(nodeTravelFilename.c_str()) != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", nodeTravelFilename.c_str());
        return;
    }

    // data set
    if(filename != NULL)
    {
        if(loadNetCdfFile(filename) != true)
        {
            put_flog(LOG_ERROR, "could not load file %s", filename);
            return;
        }

        // todo: for now, duplicate population data over all times, since this isn't stored in the data set
        for(int time=1; time<numTimes_; time++)
        {
            copyVariableToNewTimeStep("population");
        }
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

std::vector<std::string> EpidemicDataSet::getStratificationNames()
{
    if(stratificationNames_.size() == 0)
    {
        if(loadStratificationsFile() != true)
        {
            put_flog(LOG_ERROR, "could not load stratifications file");
        }
    }

    return stratificationNames_;
}

std::vector<std::vector<std::string> > EpidemicDataSet::getStratifications()
{
    if(stratificationNames_.size() == 0)
    {
        if(loadStratificationsFile() != true)
        {
            put_flog(LOG_ERROR, "could not load stratifications file");
        }
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

    return getValue("population", 0, nodeId);
}

float EpidemicDataSet::getPopulation(std::vector<int> nodeIds)
{
    float population = 0.;

    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        population += getPopulation(nodeIds[i]);
    }

    return population;
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

int EpidemicDataSet::getNodeIndex(int nodeId)
{
    return nodeIdToIndex_[nodeId];
}

std::vector<std::string> EpidemicDataSet::getVariableNames()
{
    std::vector<std::string> variableNames;

    // regular variables
    std::map<std::string, blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> >::iterator iter;

    for(iter=variables_.begin(); iter!=variables_.end(); iter++)
    {
        variableNames.push_back(iter->first);
    }

    // derived variables
    std::map<std::string, boost::function<float (int time, int nodeId, std::vector<int> stratificationValues)> >::iterator iter2;

    for(iter2=derivedVariables_.begin(); iter2!=derivedVariables_.end(); iter2++)
    {
        variableNames.push_back(iter2->first);
    }

    return variableNames;
}

std::vector<int> EpidemicDataSet::getNodeIds()
{
    return nodeIds_;
}

std::vector<int> EpidemicDataSet::getNodeIds(std::string groupName)
{
    if(groupNameToNodeIds_.count(groupName) == 0)
    {
        put_flog(LOG_ERROR, "could not map group name %s to node ids", groupName.c_str());
        return std::vector<int>();
    }

    return groupNameToNodeIds_[groupName];
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

float EpidemicDataSet::getTravel(int nodeId0, int nodeId1)
{
    if(nodeIdToIndex_.count(nodeId0) == 0 || nodeIdToIndex_.count(nodeId1) == 0)
    {
        put_flog(LOG_ERROR, "could not map a nodeId to an index for: %i, %i", nodeId0, nodeId1);
        return 0.;
    }

    return travel_(nodeIdToIndex_[nodeId0], nodeIdToIndex_[nodeId1]);
}

float EpidemicDataSet::getValue(const std::string &varName, const int &time, const int &nodeId, const std::vector<int> &stratificationValues)
{
    // handle derived variables
    if(derivedVariables_.count(varName) > 0)
    {
        return derivedVariables_[varName](time, nodeId, stratificationValues);
    }

    if(variables_.count(varName) == 0)
    {
        put_flog(LOG_ERROR, "no such variable %s (nodeId = %i)", varName.c_str(), nodeId);
        return 0.;
    }
    else if(nodeId != NODES_ALL && nodeIdToIndex_.count(nodeId) == 0)
    {
        put_flog(LOG_ERROR, "could not map nodeId %i to an index (varName = %s)", nodeId, varName.c_str());
        return 0.;
    }

    // the variable we're getting
    blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> variable = variables_[varName];

    // the full domain
    blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> lowerBound = variable.lbound();
    blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> upperBound = variable.ubound();

    // make sure this variable is valid for the specified time
    if(time < lowerBound(0) || time > upperBound(0))
    {
        put_flog(LOG_WARN, "variable %s not valid for time %i", varName.c_str(), time);
        return 0.;
    }

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

    // return the sum of the array over the subdomain
    return blitz::sum(variable(subdomain));
}

float EpidemicDataSet::getValue(const std::string &varName, const int &time, const int &nodeId, const std::vector<std::vector<int> > &stratificationValuesSet)
{
    float value = 0.;

    for(unsigned int i=0; i<stratificationValuesSet.size(); i++)
    {
        value += getValue(varName, time, nodeId, stratificationValuesSet[i]);
    }

    return value;
}

float EpidemicDataSet::getValue(const std::string &varName, const int &time, const std::string &groupName, const std::vector<int> &stratificationValues)
{
    if(variables_.count(varName) == 0 && derivedVariables_.count(varName) == 0)
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

bool EpidemicDataSet::newVariable(std::string varName)
{
    if(variables_.count(varName) != 0)
    {
        put_flog(LOG_ERROR, "variable %s already exists", varName.c_str());
        return false;
    }

    // full shape
    blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> shape;
    shape(0) = numTimes_;
    shape(1) = numNodes_;

    for(int j=0; j<NUM_STRATIFICATION_DIMENSIONS; j++)
    {
        shape(2 + j) = stratifications_[j].size();
    }

    // create the variable
    blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> var(shape);

    // initialize values to zero
    var = 0.;

    // add the variable to the vector
    variables_[varName].reference(var);

    return true;
}

bool EpidemicDataSet::copyVariable(std::string sourceVarName, std::string destVarName)
{
    if(variables_.count(sourceVarName) == 0)
    {
        put_flog(LOG_ERROR, "no such variable %s", sourceVarName.c_str());
        return false;
    }

    if(variables_.count(destVarName) != 0)
    {
        put_flog(LOG_ERROR, "destination variable %s already exists", destVarName.c_str());
        return false;
    }

    blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> varCopy = variables_[sourceVarName].copy();

    variables_[destVarName].reference(varCopy);

    return true;
}

bool EpidemicDataSet::copyVariableToNewTimeStep(std::string varName)
{
    if(variables_.count(varName) == 0)
    {
        put_flog(LOG_ERROR, "no such variable %s", varName.c_str());
        return false;
    }

    // get current shape of variable
    blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> shape = variables_[varName].shape();

    // add a time step
    shape(0) = shape(0) + 1;

    // resize variable for the new shape
    variables_[varName].resizeAndPreserve(shape);

    // copy data to the new time step

    // the full domain
    blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> lowerBound = variables_[varName].lbound();
    blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> upperBound = variables_[varName].ubound();

    // domain for previous time and new time
    lowerBound(0) = upperBound(0) = variables_[varName].extent(0) - 2;
    blitz::RectDomain<2+NUM_STRATIFICATION_DIMENSIONS> subdomain0(lowerBound, upperBound);

    lowerBound(0) = upperBound(0) = variables_[varName].extent(0) - 1;
    blitz::RectDomain<2+NUM_STRATIFICATION_DIMENSIONS> subdomain1(lowerBound, upperBound);

    // make the copy
    variables_[varName](subdomain1) = variables_[varName](subdomain0);

    return true;
}

blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> EpidemicDataSet::getVariableAtTime(std::string varName, int time)
{
    if(variables_.count(varName) == 0)
    {
        put_flog(LOG_ERROR, "no such variable %s", varName.c_str());
        return blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS>();
    }

    if(time >= variables_[varName].extent(0))
    {
        put_flog(LOG_ERROR, "time %i >= time extent %i", time, variables_[varName].extent(0));
        return blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS>();
    }

    // this should produce a slice that references the data in the original variable array
    blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> subVar = variables_[varName](time, blitz::Range::all(), BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, TEXT, blitz::Range::all()));

    return subVar;
}

blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> EpidemicDataSet::getVariableAtFinalTime(std::string varName)
{
    if(variables_.count(varName) == 0)
    {
        put_flog(LOG_ERROR, "no such variable %s", varName.c_str());
        return blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS>();
    }

    // this should produce a slice that references the data in the original variable array
    int finalTime = variables_[varName].extent(0) - 1;

    blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> subVar = variables_[varName](finalTime, blitz::Range::all(), BOOST_PP_ENUM(NUM_STRATIFICATION_DIMENSIONS, TEXT, blitz::Range::all()));

    return subVar;
}

boost::shared_ptr<StockpileNetwork> EpidemicDataSet::getStockpileNetwork()
{
    return stockpileNetwork_;
}

std::string EpidemicDataSet::getVariableSummaryNodeVsTime(const std::string &varName)
{
    if(variables_.count(varName) == 0)
    {
        put_flog(LOG_ERROR, "no such variable %s", varName.c_str());
        return std::string();
    }

    // nodeIds
    std::vector<int> nodeIds = getNodeIds();

    // generate the CSV
    std::stringstream out;

    // set maximum decimal precision for stringstream
    out.precision(16);

    // header
    out << "t";

    // header: for each node
    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        out << "," << nodeIds[i];
    }

    out << std::endl;

    // row for each time
    for(unsigned t=0; t<getNumTimes(); t++)
    {
        out << t;

        for(unsigned int n=0; n<nodeIds.size(); n++)
        {
            out << "," << getValue(varName, t, nodeIds[n]);
        }

        out << std::endl;
    }

    return out.str();
}

bool EpidemicDataSet::loadNetCdfFile(const char * filename)
{
#if USE_NETCDF // TODO: should handle this differently
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

    // make sure we have the expected number of nodes
    if(nodesDim->size() != numNodes_)
    {
        put_flog(LOG_FATAL, "got %i nodes, expected %i", nodesDim->size(), numNodes_);
        return false;
    }

    put_flog(LOG_DEBUG, "file contains %i timesteps, %i nodes", numTimes_, numNodes_);

    // make sure number of stratifications matches our expectation...
    int numExpectedStratifications = 1;

    for(unsigned int i=0; i<NUM_STRATIFICATION_DIMENSIONS; i++)
    {
        numExpectedStratifications *= stratifications_[i].size();
    }

    if(stratificationsDim->size() != numExpectedStratifications)
    {
        put_flog(LOG_FATAL, "got %i stratifications, expected %i", stratificationsDim->size(), numExpectedStratifications);
        return false;
    }

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
#endif
    return true;
}

bool EpidemicDataSet::loadStratificationsFile()
{
    std::string filename = g_dataDirectory + "/" + STRATIFICATIONS_FILENAME;

    std::ifstream in(filename.c_str());

    if(in.is_open() != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", filename.c_str());
        return false;
    }

    // clear existing entries
    stratificationNames_.clear();
    stratifications_.clear();

    // use boost tokenizer to parse the file
    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

    std::vector<std::string> vec;
    std::string line;

    // read names of stratifications
    getline(in, line);
    Tokenizer tok(line);
    vec.assign(tok.begin(), tok.end());

    // make sure this matches what we expect
    if(vec.size() != NUM_STRATIFICATION_DIMENSIONS)
    {
        put_flog(LOG_ERROR, "got %i stratification dimensions, expected %i", vec.size(), NUM_STRATIFICATION_DIMENSIONS);
        return false;
    }

    stratificationNames_ = vec;

    // read stratification value names
    for(unsigned int i=0; i<NUM_STRATIFICATION_DIMENSIONS; i++)
    {
        vec.clear();

        getline(in, line);
        Tokenizer tok(line);
        vec.assign(tok.begin(), tok.end());

        stratifications_.push_back(vec);
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
    numNodes_ = 0;
    nodeIds_.clear();
    nodeIdToIndex_.clear();
    nodeIdToName_.clear();
    nodeIdToGroupName_.clear();
    groupNameToNodeIds_.clear();

    // use boost tokenizer to parse the file
    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

    std::vector<std::string> vec;
    std::string line;

    // read (and ignore) header
    getline(in, line);

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

        int nodeId = atoi(vec[0].c_str());

        // nodeId vector
        nodeIds_.push_back(nodeId);

        // nodeId -> index mapping
        nodeIdToIndex_[nodeId] = index;

        // nodeId -> name mapping
        nodeIdToName_[nodeId] = vec[1];

        // nodeId -> group name mapping
        nodeIdToGroupName_[nodeId] = vec[2];

        // group name -> nodeIds indexing
        groupNameToNodeIds_[vec[2]].push_back(nodeId);

        index++;
    }

    numNodes_ = index;

    return true;
}

bool EpidemicDataSet::loadNodePopulationFile(const char * filename)
{
    // make sure we have appropriate number of stratifications
    if(stratifications_.size() < 1)
    {
        put_flog(LOG_ERROR, "need at least 1 stratification, got %i", stratifications_.size());
        return false;
    }

    std::ifstream in(filename);

    if(in.is_open() != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", filename);
        return false;
    }

    // full shape of population variable: [time][node][stratifications...]
    blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> shape;
    shape(0) = 1; // one time step
    shape(1) = numNodes_;

    for(int j=0; j<NUM_STRATIFICATION_DIMENSIONS; j++)
    {
        shape(2 + j) = stratifications_[j].size();
    }

    blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> population(shape);

    population = 0.;

    // the data file contains population data stratified only by the first stratification
    // stratification values of 0 are assumed for all other stratifications

    // use boost tokenizer to parse the file
    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

    std::vector<std::string> vec;
    std::string line;

    // read (and ignore) header
    getline(in, line);

    while(getline(in, line))
    {
        Tokenizer tok(line);

        vec.assign(tok.begin(), tok.end());

        if(vec.size() != 1+stratifications_[0].size())
        {
            put_flog(LOG_ERROR, "number of values != %i, == %i", 1+stratifications_[0].size(), vec.size());
            return false;
        }

        int time = 0;
        int nodeId = atoi(vec[0].c_str());

        if(nodeIdToIndex_.count(nodeId) == 0)
        {
            put_flog(LOG_ERROR, "could not map nodeId %i to an index", nodeId);
            return false;
        }

        int nodeIndex = nodeIdToIndex_[nodeId];

        for(int i=0; i<(int)stratifications_[0].size(); i++)
        {
            // array position; all indices initialized to 0
            blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> index(0);

            index(0) = time;
            index(1) = nodeIndex;
            index(2) = i;

            // all other stratification indices are zero

            population(index) = atof(vec[1+i].c_str());
        }
    }

    // add to regular variables map
    variables_["population"].reference(population);

    return true;
}

bool EpidemicDataSet::loadNodePopulationSecondStratificationFile(const char * filename)
{
    // load data file indicating fractional split for the second stratification
    // stratification values of 0 are assumed for all subsequent stratifications

    // make sure we have appropriate number of stratifications
    if(stratifications_.size() < 2)
    {
        put_flog(LOG_ERROR, "need at least 2 stratifications, got %i", stratifications_.size());
        return false;
    }

    // the second stratification should be of size 2
    if(stratifications_[1].size() != 2)
    {
        put_flog(LOG_ERROR, "expected 2 stratifications, got %i", stratifications_[1].size());
        return false;
    }

    std::ifstream in(filename);

    if(in.is_open() != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", filename);
        return false;
    }

    // use boost tokenizer to parse the file
    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

    std::vector<std::string> vec;
    std::string line;

    // read (and ignore) header
    getline(in, line);

    // read data (one line)
    getline(in, line);

    Tokenizer tok(line);

    vec.assign(tok.begin(), tok.end());

    if(vec.size() != stratifications_[0].size())
    {
        put_flog(LOG_ERROR, "number of values != %i, == %i", stratifications_[0].size(), vec.size());
        return false;
    }

    int time = 0;

    for(int i=0; i<(int)nodeIds_.size(); i++)
    {
        int nodeId = nodeIds_[i];

        if(nodeIdToIndex_.count(nodeId) == 0)
        {
            put_flog(LOG_ERROR, "could not map nodeId %i to an index", nodeId);
            return false;
        }

        int nodeIndex = nodeIdToIndex_[nodeId];

        for(int j=0; j<(int)stratifications_[0].size(); j++)
        {
            // get total value over the first stratification
            std::vector<int> stratificationValues(NUM_STRATIFICATION_DIMENSIONS, STRATIFICATIONS_ALL);
            stratificationValues[0] = j;

            float total = getValue("population", time, nodeId, stratificationValues);

            float fraction0 = atof(vec[j].c_str());

            float value0 = total * fraction0;
            float value1 = total * (1. - fraction0);

            // array position; all indices initialized to 0
            blitz::TinyVector<int, 2+NUM_STRATIFICATION_DIMENSIONS> index(0);

            index(0) = time;
            index(1) = nodeIndex;
            index(2) = j;

            index(3) = 0;
            variables_["population"](index) = value0;

            index(3) = 1;
            variables_["population"](index) = value1;
        }
    }

    return true;
}

bool EpidemicDataSet::loadNodeTravelFile(const char * filename)
{
    std::ifstream in(filename);

    if(in.is_open() != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", filename);
        return false;
    }

    // full shape of travel variable: [node][node]
    blitz::TinyVector<float, 2> shape;
    shape(0) = numNodes_;
    shape(1) = numNodes_;

    blitz::Array<float, 2> travel(shape);

    travel = 0.;

    // use boost tokenizer to parse the file
    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

    std::vector<std::string> vec;
    std::string line;

    // read (and ignore) header
    getline(in, line);

    int index = 0;

    while(getline(in, line))
    {
        Tokenizer tok(line);

        vec.assign(tok.begin(), tok.end());

        if((int)vec.size() != numNodes_)
        {
            put_flog(LOG_ERROR, "number of values != %i, == %i", numNodes_, vec.size());
            return false;
        }

        for(int i=0; i<(int)vec.size(); i++)
        {
            travel(index, i) = atof(vec[i].c_str());
        }

        index++;
    }

    // we should have read numNodes_ lines
    if(index != numNodes_)
    {
        put_flog(LOG_ERROR, "expected %i lines, read %i", numNodes_, index);
        return false;
    }

    travel_.reference(travel);

    return true;
}
