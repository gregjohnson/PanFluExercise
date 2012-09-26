#include "EpidemicDataSet.h"
#include <netcdfcpp.h>
#include "log.h"

EpidemicDataSet::EpidemicDataSet(const char * filename)
{
    // defaults
    isValid_ = false;

    if(loadNetCdfFile(filename) != true)
    {
        put_flog(LOG_ERROR, "could not load file %s", filename);
        return;
    }
    else
    {
        isValid_ = true;
    }
}

bool EpidemicDataSet::isValid()
{
    return isValid_;
}

bool EpidemicDataSet::loadNetCdfFile(const char * filename)
{
    // change netcdf library error behavior
    NcError err(NcError::verbose_nonfatal);

    // open the netcdf file
    NcFile ncFile(filename, NcFile::ReadOnly);

    if(!ncFile.is_valid())
    {
        put_flog(LOG_FATAL, "invalid file %s", filename);
        return false;
    }

    // get dimensions
    NcDim * timeDim = ncFile.get_dim("time");
    NcDim * nodesDim = ncFile.get_dim("nodes");
    NcDim * stratificationsDim = ncFile.get_dim("stratifications");

    if(timeDim == NULL || nodesDim == NULL || stratificationsDim == NULL)
    {
        put_flog(LOG_FATAL, "could not find a required dimension");
        return false;
    }

    int numTimes = timeDim->size();
    int numNodes = nodesDim->size();
    int numStratifications = stratificationsDim->size();

    put_flog(LOG_DEBUG, "file contains %i timesteps, %i nodes, %i stratifications", numTimes, numNodes, numStratifications);

    // get the required variables
    NcVar * ncVarIds = ncFile.get_var("ids_data");
    NcVar * ncVarPopulation = ncFile.get_var("population_data");
    NcVar * ncVarTravel = ncFile.get_var("travel_data");

    if(!ncVarIds->is_valid() || !ncVarPopulation->is_valid() || !ncVarTravel->is_valid())
    {
        put_flog(LOG_FATAL, "could not find a required variable");
        return false;
    }

    blitz::Array<int, 1> fipsIds((int *)ncVarIds->values()->base(), blitz::shape(numNodes), blitz::duplicateData);
    blitz::Array<float, 1> population((float *)ncVarPopulation->values()->base(), blitz::shape(numNodes), blitz::duplicateData);
    blitz::Array<float, 2> travel((float *)ncVarTravel->values()->base(), blitz::shape(numNodes, numNodes), blitz::duplicateData);

    fipsIds_ = fipsIds;
    population_ = population;
    travel_ = travel;

    // get all float variables with dimensions (time, nodes, stratifications)
    for(int i=0; i<ncFile.num_vars(); i++)
    {
        NcVar * ncVar = ncFile.get_var(i);

        if(ncVar->num_dims() == 3 && ncVar->type() == ncFloat && strcmp(ncVar->get_dim(0)->name(), "time") == 0 && strcmp(ncVar->get_dim(1)->name(), "nodes") == 0 && strcmp(ncVar->get_dim(2)->name(), "stratifications") == 0)
        {
            put_flog(LOG_INFO, "found variable: %s", ncVar->name());

            blitz::Array<float, 3> var((float *)ncVar->values()->base(), blitz::shape(numTimes, numNodes, numStratifications), blitz::duplicateData);

            variables_[std::string(ncVar->name())] = var;
        }
    }

    return true;
}
