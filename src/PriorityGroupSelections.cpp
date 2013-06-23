#include "PriorityGroupSelections.h"
#include "PriorityGroup.h"

PriorityGroupSelections::PriorityGroupSelections(std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups)
{
    priorityGroups_ = priorityGroups;
}

std::vector<boost::shared_ptr<PriorityGroup> > PriorityGroupSelections::getPriorityGroups()
{
    return priorityGroups_;
}
