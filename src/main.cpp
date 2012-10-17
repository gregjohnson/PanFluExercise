#include "main.h"
#include "MainWindow.h"
#include "log.h"
#include <QtGui>

QApplication * g_app = NULL;
MainWindow * g_mainWindow = NULL;
std::string g_dataDirectory;

int main(int argc, char * argv[])
{
    g_app = new QApplication(argc, argv);

    // get directory of application
    QDir appDirectory = QDir(QCoreApplication::applicationDirPath());

    // and data directory
    QDir dataDirectory = appDirectory;

#ifdef __APPLE__
    dataDirectory.cdUp();
    dataDirectory.cd("Resources");
    dataDirectory.cd("data");
#endif

    g_dataDirectory = dataDirectory.absolutePath().toStdString();

    put_flog(LOG_DEBUG, "data directory: %s", g_dataDirectory.c_str());

    g_mainWindow = new MainWindow();

    // enter Qt event loop
    g_app->exec();

    return 0;
}
