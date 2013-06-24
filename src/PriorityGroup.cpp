#include "PriorityGroup.h"
#include "EpidemicDataSet.h"

PriorityGroup::PriorityGroup(std::string name, std::vector<std::vector<int> > stratificationVectorValues)
{
    name_ = name;

    // check for and replace STRATIFICATIONS_ALL values
    // this allows us (for example in PriorityGroupSelections) to determine non-overlapping sets
    for(unsigned int i=0; i<stratificationVectorValues.size(); i++)
    {
        for(unsigned int j=0; j<stratificationVectorValues[i].size(); j++)
        {
            if(stratificationVectorValues[i][j] == STRATIFICATIONS_ALL)
            {
                // replace the vector with all the individual entries
                stratificationVectorValues[i].clear();

                std::vector<std::vector<std::string> > stratifications = EpidemicDataSet::getStratifications();

                for(unsigned int k=0; k<stratifications[i].size(); k++)
                {
                    stratificationVectorValues[i].push_back(k);
                }

                break;
            }
        }
    }

    stratificationVectorValues_ = stratificationVectorValues;
}

std::string PriorityGroup::getName()
{
    return name_;
}

std::vector<std::vector<int> > PriorityGroup::getStratificationVectorValues()
{
    return stratificationVectorValues_;
}
