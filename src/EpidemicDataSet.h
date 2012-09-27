#ifndef EPIDEMIC_DATA_SET_H
#define EPIDEMIC_DATA_SET_H

#include <map>
#include <vector>
#include <blitz/array.h>

#define NODES_ALL -1

class EpidemicDataSet
{
    public:

        EpidemicDataSet(const char * filename);

        bool isValid();

        int getNumTimes();
        int getNumNodes();
        int getNumStratifications();

        float getPopulation(int nodeId);

        std::vector<std::string> getVariableNames();

        float getValue(std::string varName, int time, int nodeId);

    private:

        bool isValid_;

        // dimensionality
        int numTimes_;
        int numNodes_;
        int numStratifications_;

        // special variables
        blitz::Array<int, 1> fipsIds_;
        blitz::Array<float, 1> population_;
        blitz::Array<float, 2> travel_;

        // maps node id to array index
        std::map<int, int> nodeIdToIndex_;

        // all regular variables
        std::map<std::string, blitz::Array<float, 3> > variables_;

        bool loadNetCdfFile(const char * filename);
};

#endif
