#ifndef PRIORITY_GROUP_SELECTIONS_H
#define PRIORITY_GROUP_SELECTIONS_H

#include "EpidemicDataSet.h"
#include <boost/shared_ptr.hpp>
#include <vector>

class PriorityGroup;

class PriorityGroupSelections
{
    public:

        PriorityGroupSelections(std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups);

        std::vector<boost::shared_ptr<PriorityGroup> > getPriorityGroups();

        // get the full set of stratification values, only considering the first 2 indices.
        // the 3rd index will be populated with thirdIndexValue.
        // this returns a unique non-overlapping set of stratification values.
        std::vector<std::vector<int> > getStratificationValuesSet2(int thirdIndexValue=STRATIFICATIONS_ALL);

        // get the full set of stratification values, considering all indices.
        // this returns a unique non-overlapping set of stratification values.
        std::vector<std::vector<int> > getStratificationValuesSet();

    private:

        std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups_;
};

#endif
