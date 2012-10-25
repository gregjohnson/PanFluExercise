#ifndef EPIDEMIC_DATA_SET_H
#define EPIDEMIC_DATA_SET_H

#include <map>
#include <vector>
#include <blitz/array.h>

// must be defined at compile time, and match definition of EpidemicDataSet::stratificationNames_, stratifications_
// stratifications: [age group][risk group][vaccinated]
#define NUM_STRATIFICATION_DIMENSIONS 3

#define STRATIFICATIONS_ALL -1

#define NODES_ALL -1

class EpidemicDataSet
{
    public:

        EpidemicDataSet(const char * filename);

        bool isValid();

        int getNumTimes();
        int getNumNodes();
        int getNumStratifications();

        static std::vector<std::string> getStratificationNames();
        static std::vector<std::vector<std::string> > getStratifications();

        float getPopulation(int nodeId);
        std::string getNodeName(int nodeId);

        std::vector<int> getNodeIds();
        std::vector<std::string> getGroupNames();
        std::vector<std::string> getVariableNames();

        float getValue(std::string varName, int time, int nodeId, std::vector<int> stratificationValues=std::vector<int>());
        float getValue(std::string varName, int time, std::string groupName, std::vector<int> stratificationValues=std::vector<int>());

    private:

        bool isValid_;

        // dimensionality
        int numTimes_;
        int numNodes_;
        int numStratifications_;

        // stratification details; for now these need to be hardcoded
        static std::vector<std::string> stratificationNames_;
        static std::vector<std::vector<std::string> > stratifications_;

        // special variables
        blitz::Array<float, 1> population_;
        blitz::Array<float, 2> travel_;

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

        // all regular variables
        std::map<std::string, blitz::Array<float, 2+NUM_STRATIFICATION_DIMENSIONS> > variables_;

        bool loadNetCdfFile(const char * filename);
        bool loadNodeNameGroupFile(const char * filename);
};

#endif
