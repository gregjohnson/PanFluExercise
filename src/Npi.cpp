#include "Npi.h"

Npi::Npi(std::string name)
{
    name_ = name;
}

std::string Npi::getName()
{
    return name_;
}
