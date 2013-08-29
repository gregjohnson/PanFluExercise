#include "iliView.h"
#include "../../main.h"
#include "../MersenneTwister.h"
#include "../../log.h"
#include <iostream>
#include <fstream>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

// random number generators
MTRand iliRand;
gsl_rng * iliRandGenerator;

// ILI noise data
std::vector<float> iliNoiseVector;

// misc
std::vector<int> repeat(int number, int times);
std::vector<float> repeat(float number, int times);

// only used in  iliInit()
std::vector<float> getProviderStartStopProbabilities(std::string filename, int numProviders);

// used on every call to iliView()
int doesReport(int prevStatus, float restart, float restop);
std::vector<int> oneStep(std::vector<int> prevStatus, std::vector<float> start, std::vector<float> stop);
float average(std::vector<float> epi, std::vector<int> status);

/*
example for stand-alone version:

int main()
{
    // making up some data
    std::vector<float> epi = repeat((float)10., 254);
    std::vector<float> pops = repeat((float)100., 254);
    
    // intializing
    std::vector<Provider> providers = iliInit();
    
    // filtering
    std::vector<float> filtered = iliView(epi, pops, providers);

    for(unsigned int i=0; i<filtered.size(); i++)
    {
        std::cout << filtered[i] << ", ";
    }

    std::cout << std::endl;

    return 0;
}
*/

std::vector<int> repeat(int number, int times)
{
    std::vector<int> vec;

    for(int i=0; i<times; i++)
    {
        vec.push_back(number);
    }

    return vec;
}

std::vector<float> repeat(float number, int times)
{
    std::vector<float> vec;

    for(int i=0; i<times; i++)
    {
        vec.push_back(number);
    }

    return vec;
}

std::vector<float> getProviderStartStopProbabilities(std::string filename, int numProviders)
{
    std::ifstream ifs(filename.c_str());

    std::vector<float> vec;

    float n;

    while(ifs >> n)
    {
        vec.push_back(n);
    }

    ifs.close();

    std::vector<float> outVec;

    if(numProviders > 0)
    {
        int counter = 0;

        while(counter < numProviders)
        {
            int selected = iliRand.randInt(vec.size()-1);
            outVec.push_back(vec[selected]);
            counter++;
        }
    }

    return outVec;
}

int doesReport(int prevStatus, float restart, float restop)
{
    int numberRestart = (int)gsl_ran_binomial(iliRandGenerator, restart, 1);
    int numberRestop = (int)gsl_ran_binomial(iliRandGenerator, restop, 1);

    int reportStatus;

    if(prevStatus == 1)
        reportStatus = numberRestop;
    else
        reportStatus = numberRestart;

    return reportStatus;
}

std::vector<int> oneStep(std::vector<int> prevStatus, std::vector<float> start, std::vector<float> stop)
{
    std::vector<int> doesRepi;

    for(int i=0; i<prevStatus.size(); i++)
    {
        doesRepi.push_back(doesReport(prevStatus[i], start[i], stop[i]));
    }

    return(doesRepi);
}

float average(std::vector<float> epi, std::vector<int> status)
{
    float sum = 0.;
    float counter = 0.;

    for(int i=0; i<epi.size(); i++)
    {
        int selected = iliRand.randInt(iliNoiseVector.size()-1);

        float noisei = iliRand.randNorm(0., iliNoiseVector[selected]);

        float reporti = epi[i]+noisei;

        if(reporti < 0.)
            reporti = 0.0;
    
        sum += reporti*status[i];
        counter += 1.;
    }

    return sum / counter;
}

std::vector<Provider> iliInit()
{
    // setup random number generators
    gsl_rng_env_setup();
    iliRandGenerator = gsl_rng_alloc(gsl_rng_default);

    std::ifstream ifs((g_dataDirectory + "/ILI/numCountyProviders.txt").c_str());

    std::vector<int> numProviders;

    int n;

    while(ifs >> n)
    {
        numProviders.push_back(n);
    }

    ifs.close();

    std::vector<Provider> providers;

    for(int i=0; i<numProviders.size(); i++)
    {
        Provider provider;
        provider.starts = getProviderStartStopProbabilities(g_dataDirectory + "/ILI/providerStartProbabilities.txt", numProviders[i]);
        provider.stops = getProviderStartStopProbabilities(g_dataDirectory + "/ILI/providerStopProbabilities.txt", numProviders[i]);
        provider.status = repeat(1, numProviders[i]);

        providers.push_back(provider);
    }

    // also, initialize ILI noise data
    iliNoiseVector.clear();

    std::ifstream ifs2((g_dataDirectory + "/ILI/providerNoiseData.txt").c_str());

    float n2;

    while(ifs2 >> n2)
    {
        iliNoiseVector.push_back(n2);
    }

    ifs2.close();

    return(providers);
}

std::vector<float> iliView(std::vector<float> epi, std::vector<float> pop, std::vector<Provider> &providers)
{
    std::vector<float> iliViewOut;

    for(int i=0; i<epi.size(); i++)
    {
        if(providers[i].status.size() > 0)
        {
            std::vector<float> epii = repeat(epi[i], providers[i].starts.size());
            std::vector<int> statusi = oneStep(providers[i].status, providers[i].starts, providers[i].stops);

            float repi = average(epii, statusi) / pop[i];

            iliViewOut.push_back(repi);

            // update status in provider
            providers[i].status = statusi;
        }
        else
        {
            iliViewOut.push_back(0.0);
        }
    }

    return iliViewOut;
}
