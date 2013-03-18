#include "main.h"
#include "MainWindow.h"
#include "log.h"
#include <QtGui>
#include <QtNetwork/QTcpSocket>
#include <vtkObject.h>

MainWindow * g_mainWindow = NULL;
std::string g_dataDirectory;
DcSocket * g_dcSocket = NULL;

int main(int argc, char * argv[])
{
    QApplication * app = new QApplication(argc, argv);

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

    // disable VTK console messages
    vtkObject::GlobalWarningDisplayOff();

    g_mainWindow = new MainWindow();

    // enter Qt event loop
    app->exec();

    delete g_mainWindow;

    return 0;
}

bool sendSVGToDisplayCluster(std::string filename, std::string name)
{
    if(g_dcSocket == NULL || g_dcSocket->state() != QAbstractSocket::ConnectedState)
    {
        put_flog(LOG_ERROR, "error sending SVG stream, socket not connected");

        dcStreamDisconnect(g_dcSocket);
        g_dcSocket = NULL;

        QMessageBox messageBox;
        messageBox.setText("Lost connection to DisplayCluster.");
        messageBox.exec();

        return false;
    }

    // open file corresponding to URI
    QFile file(filename.c_str());

    if(file.open(QIODevice::ReadOnly) != true)
    {
        put_flog(LOG_ERROR, "could not open file %s", filename.c_str());
        return false;
    }

    QByteArray imageData = file.readAll();

    bool success = dcStreamSendSVG(g_dcSocket, name, imageData.constData(), imageData.size());

    if(success != true)
    {
        put_flog(LOG_ERROR, "error sending SVG stream");
    }

    return success;
}
