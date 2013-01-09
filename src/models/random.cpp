#include "random.h"
#include "MersenneTwister.h"
#include <cmath>

double random_exponential(double lambda, MTRand * rand)
{
    return -log(rand->rand()) / lambda;
}
