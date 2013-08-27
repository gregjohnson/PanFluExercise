#ifndef NPI_H
#define NPI_H

#include <string>

class Npi
{
    public:

        Npi(std::string name);

        std::string getName();

    private:

        std::string name_;
};

#endif
