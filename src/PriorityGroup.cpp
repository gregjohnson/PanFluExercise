#include "PriorityGroup.h"

PriorityGroup::PriorityGroup(std::string name, std::vector<std::vector<int> > stratificationVectorValues)
{
    name_ = name;
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
