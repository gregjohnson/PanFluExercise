#include "PriorityGroupSelections.h"
#include "PriorityGroup.h"
#include "log.h"
#include <set>

PriorityGroupSelections::PriorityGroupSelections(std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups)
{
    priorityGroups_ = priorityGroups;
}

std::vector<boost::shared_ptr<PriorityGroup> > PriorityGroupSelections::getPriorityGroups()
{
    return priorityGroups_;
}

std::vector<std::vector<int> > PriorityGroupSelections::getStratificationValuesSet2(int thirdIndexValue)
{
    // use a set container to only keep unique stratification values entries
    std::set<std::vector<int> > set;

    for(unsigned int i=0; i<priorityGroups_.size(); i++)
    {
        std::vector<std::vector<int> > svv = priorityGroups_[i]->getStratificationVectorValues();

        for(unsigned int j0=0; j0<svv[0].size(); j0++)
        {
            for(unsigned int j1=0; j1<svv[1].size(); j1++)
            {
                std::vector<int> stratificationValues;

                // warn if we find STRATIFICATIONS_ALL values
                // we want a non-overlapping set of stratification values -- using STRATIFICATIONS_ALL doesn't allow us to preserve that
                if(svv[0][j0] == STRATIFICATIONS_ALL || svv[1][j1] == STRATIFICATIONS_ALL)
                {
                    put_flog(LOG_WARN, "found unexpected STRATIFICATIONS_ALL value!");
                }

                stratificationValues.push_back(svv[0][j0]);
                stratificationValues.push_back(svv[1][j1]);

                // forcing 3rd index
                stratificationValues.push_back(thirdIndexValue);

                set.insert(stratificationValues);
            }
        }
    }

    // convert the set to a vector for simplicity
    std::vector<std::vector<int> > vector(set.begin(), set.end());

    return vector;
}

std::vector<std::vector<int> > PriorityGroupSelections::getStratificationValuesSet()
{
    // use a set container to only keep unique stratification values entries
    std::set<std::vector<int> > set;

    for(unsigned int i=0; i<priorityGroups_.size(); i++)
    {
        std::vector<std::vector<int> > svv = priorityGroups_[i]->getStratificationVectorValues();

        for(unsigned int j0=0; j0<svv[0].size(); j0++)
        {
            for(unsigned int j1=0; j1<svv[1].size(); j1++)
            {
                for(unsigned int j2=0; j2<svv[2].size(); j2++)
                {
                    std::vector<int> stratificationValues;

                    // warn if we find STRATIFICATIONS_ALL values
                    // we want a non-overlapping set of stratification values -- using STRATIFICATIONS_ALL doesn't allow us to preserve that
                    if(svv[0][j0] == STRATIFICATIONS_ALL || svv[1][j1] == STRATIFICATIONS_ALL || svv[2][j2] == STRATIFICATIONS_ALL)
                    {
                        put_flog(LOG_WARN, "found unexpected STRATIFICATIONS_ALL value!");
                    }

                    stratificationValues.push_back(svv[0][j0]);
                    stratificationValues.push_back(svv[1][j1]);
                    stratificationValues.push_back(svv[2][j2]);

                    set.insert(stratificationValues);
                }
            }
        }
    }

    // convert the set to a vector for simplicity
    std::vector<std::vector<int> > vector(set.begin(), set.end());

    return vector;
}
