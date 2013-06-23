#ifndef PRIORITY_GROUP_SELECTIONS_H
#define PRIORITY_GROUP_SELECTIONS_H

#include <boost/shared_ptr.hpp>
#include <vector>

class PriorityGroup;

class PriorityGroupSelections
{
    public:

        PriorityGroupSelections(std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups);

        std::vector<boost::shared_ptr<PriorityGroup> > getPriorityGroups();

    private:

        std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups_;
};

#endif
