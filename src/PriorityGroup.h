#ifndef PRIORITY_GROUP_H
#define PRIORITY_GROUP_H

#include <string>
#include <vector>

class PriorityGroup
{
    public:

        PriorityGroup(std::string name, std::vector<std::vector<int> > stratificationVectorValues);

        std::string getName();
        std::vector<std::vector<int> > getStratificationVectorValues();

    private:

        std::string name_;
        std::vector<std::vector<int> > stratificationVectorValues_;
};

#endif
