#ifndef MAIN_H
#define MAIN_H

#include <string>

class MainWindow;

extern bool g_batchMode;
extern int g_batchNumTimesteps;
extern std::string g_batchInitialCasesFilename;
extern std::string g_batchParametersFilename;
extern std::string g_batchOutputVariable;
extern std::string g_batchOutputFilename;

extern MainWindow * g_mainWindow;
extern std::string g_dataDirectory;

#if USE_DISPLAYCLUSTER
    #include "dcStream.h"

    extern DcSocket * g_dcSocket;
    extern bool sendSVGToDisplayCluster(std::string filename, std::string name);
#endif

#endif
