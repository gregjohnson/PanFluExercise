#include "Stockpile.h"
#include "log.h"

Stockpile::Stockpile(std::string name)
{
    name_ = name;
    num_.push_back(0);
}

std::string Stockpile::getName()
{
    return name_;
}

int Stockpile::getNum(int time)
{
    if(time >= (int)num_.size())
    {
        put_flog(LOG_ERROR, "time %i >= %i", time, num_.size());
        return 0;
    }

    return num_[time];
}

void Stockpile::setNodeIds(std::vector<int> nodeIds)
{
    nodeIds_ = nodeIds;
}

std::vector<int> Stockpile::getNodeIds()
{
    return nodeIds_;
}

void Stockpile::copyToNewTimeStep()
{
    num_.push_back(num_.back());
}

void Stockpile::setNum(int time, int num)
{
    if(time >= (int)num_.size())
    {
        put_flog(LOG_ERROR, "time %i >= %i", time, num_.size());
        return;
    }

    num_[time] = num;
}
