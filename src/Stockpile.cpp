#include "Stockpile.h"
#include "log.h"

Stockpile::Stockpile(std::string name)
{
    name_ = name;

    boost::array<int, NUM_STOCKPILE_TYPES> zeros;
    zeros.fill(0);

    num_.push_back(zeros);
}

std::string Stockpile::getTypeName(STOCKPILE_TYPE type)
{
    if(type == STOCKPILE_ANTIVIRALS)
    {
        return std::string("Antivirals");
    }
    else if(type == STOCKPILE_VACCINES)
    {
        return std::string("Vaccines");
    }
    else
    {
        return std::string("Unknown");
    }
}

std::string Stockpile::getName()
{
    return name_;
}

int Stockpile::getNum(int time, STOCKPILE_TYPE type)
{
    if(time >= (int)num_.size())
    {
        put_flog(LOG_ERROR, "time %i >= %i", time, num_.size());
        return 0;
    }

    return num_[time][type];
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

void Stockpile::setNum(int time, int num, STOCKPILE_TYPE type)
{
    if(time >= (int)num_.size())
    {
        put_flog(LOG_ERROR, "time %i >= %i", time, num_.size());
        return;
    }

    num_[time][type] = num;
}
