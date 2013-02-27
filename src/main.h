#ifndef MAIN_H
#define MAIN_H

#include "dcStream.h"
#include <string>

class MainWindow;

extern MainWindow * g_mainWindow;
extern std::string g_dataDirectory;
extern DcSocket * g_dcSocket;

extern bool sendSVGToDisplayCluster(std::string filename, std::string name);

#endif
