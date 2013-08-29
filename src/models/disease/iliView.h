#ifndef ILI_VIEW_H
#define ILI_VIEW_H

#include <vector>

struct Provider
{
    std::vector<float> starts;
    std::vector<float> stops;
    std::vector<int> status;
};

extern std::vector<Provider> iliInit();
extern std::vector<float> iliView(std::vector<float> epi, std::vector<float> pop, std::vector<Provider> &providers);

#endif
