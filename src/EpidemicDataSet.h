#ifndef EPIDEMIC_DATA_SET_H
#define EPIDEMIC_DATA_SET_H

#include <map>
#include <blitz/array.h>

class EpidemicDataSet
{
    public:

        EpidemicDataSet(const char * filename);

        bool isValid();

    private:

        bool isValid_;

        blitz::Array<int, 1> fipsIds_;
        blitz::Array<float, 1> population_;
        blitz::Array<float, 2> travel_;

        std::map<std::string, blitz::Array<float, 3> > variables_;

        bool loadNetCdfFile(const char * filename);
};

#endif
