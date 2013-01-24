#include "Stockpile.h"
#include "log.h"

Stockpile::Stockpile(std::string name)
{
    name_ = name;
    num_ = 0;
}

std::string Stockpile::getName()
{
    return name_;
}

int Stockpile::getNum()
{
    return num_;
}

void Stockpile::setNodeIds(std::vector<int> nodeIds)
{
    nodeIds_ = nodeIds;
}

std::vector<int> Stockpile::getNodeIds()
{
    return nodeIds_;
}

void Stockpile::setNum(int num)
{
    put_flog(LOG_DEBUG, "%i", num);

    num_ = num;

    emit(changed());
}
