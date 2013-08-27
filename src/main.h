#ifndef MAIN_H
#define MAIN_H

#include <string>

class MainWindow;

extern MainWindow * g_mainWindow;
extern std::string g_dataDirectory;

#if USE_DISPLAYCLUSTER
    #include "dcStream.h"

    extern DcSocket * g_dcSocket;
    extern bool sendSVGToDisplayCluster(std::string filename, std::string name);
#endif

#endif
