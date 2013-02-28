#ifndef EPIDEMIC_DATA_SET_H
#define EPIDEMIC_DATA_SET_H

#include <map>
#include <vector>
#include <blitz/array.h>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

class StockpileNetwork;

// must be defined at compile time, and match definition in stratifications file
// stratifications: [age group][risk group][vaccinated]
#define NUM_STRATIFICATION_DIMENSIONS 3
#define STRATIFICATIONS_FILENAME "stratifications.csv"

#define STRATIFICATIONS_ALL -1

#define NODES_ALL -1

// used for argument expansion
#include <boost/preprocessor/repetition/enum.hpp>
#define TEXT(z, n, text) text
#define VECTOR_TO_ARGS(z, n, text) text[n]

class EpidemicDataSet
{
    public:

        EpidemicDataSet(const char * filename=NULL);
        virtual ~EpidemicDataSet() { }

        bool isValid();

        int getNumTimes();
        int getNumNodes();

        static std::vector<std::string> getStratificationNames();
        static std::vector<std::vector<std::string> > getStratifications();

        float getPopulation(int nodeId);
        float getPopulation(std::vector<int> nodeIds);
        std::string getNodeName(int nodeId);

        std::vector<int> getNodeIds();
        std::vector<int> getNodeIds(std::string groupName);
        std::vector<std::string> getGroupNames();
        std::vector<std::string> getVariableNames();

        float getTravel(int nodeId0, int nodeId1);

        float getValue(const std::string &varName, const int &time, const int &nodeId, const std::vector<int> &stratificationValues=std::vector<int>());
        float getValue(const std::string &varName, const int &time, const std::string &groupName, const std::vector<int> &stratificationValues=std::vector<int>());

        bool newVariable(std::string varName);
        bool copyVariable(std::string sourceVarName, std::string destVarName);
        bool copyVariableToNewTimeStep(std::string varName);

        blitz::Array<float, 1+NUM_STRATIFICATION_DIMENSIONS> getVariableAtFinalTime(std::string varName);

        boost::shared_ptr<StockpileNetwork> getStockpileNetwork();

    protected:

        bool isValid_;

        // dimensionality
        int numTimes_;
        int numNodes_;

        // stratification details
        static std::vector<std::string> stratificationNames_;
        static std::vector<std::vector<std::string> > stratifications_;

        // node id's
        std::vector<int> nodeIds_;

        // maps node id to array index
        std::map<int, int> nodeIdToIndex_;

        // maps node id to name
        std::map<int, std::string> nodeIdToName_;

        // maps node id to group name
        std::map<int, std::string> nodeIdToGroupName_;

        // maps group name to node id's
        std::map<std::string, std::vector<int> > groupNameToNodeIds_;

        // node -> node travel fractions
        blitz::Array<float, 2> travel_;

        // all regular variables
        std::map<std::string, blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> > variables_;

        // all derived variables
        std::map<std::string, boost::function<float (int time, int nodeId, std::vector<int> stratificationValues)> > derivedVariables_;

        // stockpile network
        boost::shared_ptr<StockpileNetwork> stockpileNetwork_;

        bool loadNetCdfFile(const char * filename);
        static bool loadStratificationsFile();
        bool loadNodeNameGroupFile(const char * filename);
        bool loadNodePopulationFile(const char * filename);
        bool loadNodePopulationSecondStratificationFile(const char * filename);
        bool loadNodeTravelFile(const char * filename);
};

#endif
