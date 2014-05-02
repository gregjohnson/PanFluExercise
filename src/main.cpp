#include "main.h"
#include "MainWindow.h"
#include "log.h"
#include <QtGui>
#include <QtNetwork/QTcpSocket>
#include <vtkObject.h>
#include <boost/program_options.hpp>

bool g_batchMode = false;
int g_batchNumTimesteps = 240;
std::string g_batchInitialCasesFilename;
std::string g_batchParametersFilename;
std::string g_batchOutputVariable = "deceased";
std::string g_batchOutputFilename = "deceased.csv";

MainWindow * g_mainWindow = NULL;
std::string g_dataDirectory;

#if USE_DISPLAYCLUSTER
    DcSocket * g_dcSocket = NULL;
#endif

int main(int argc, char * argv[])
{
    // parse Qt commandline options first
    QApplication * app = new QApplication(argc, argv);

    // declare the supported options
    boost::program_options::options_description programOptions("Allowed options");

    programOptions.add_options()
        ("help", "produce help message")
        ("batch", "run in batch mode")
        ("batch-numtimesteps", boost::program_options::value<int>(), "limit batch run to <n> time steps")
        ("batch-initialcasesfilename", boost::program_options::value<std::string>(), "batch mode initial cases filename")
        ("batch-parametersfilename", boost::program_options::value<std::string>(), "batch mode parameters filename")
        ("batch-outputvariable", boost::program_options::value<std::string>(), "batch output variable")
        ("batch-outputfilename", boost::program_options::value<std::string>(), "batch output filename")
    ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, programOptions), vm);
    boost::program_options::notify(vm);

    if(vm.count("help"))
    {
        std::cout << programOptions << std::endl;
        return 1;
    }

    if(vm.count("batch"))
    {
        put_flog(LOG_INFO, "enabling batch mode");

        g_batchMode = true;
    }

    if(vm.count("batch-numtimesteps"))
    {
        g_batchNumTimesteps = vm["batch-numtimesteps"].as<int>();
        put_flog(LOG_INFO, "got batch num time steps %i", g_batchNumTimesteps);
    }

    if(vm.count("batch-initialcasesfilename"))
    {
        g_batchInitialCasesFilename = vm["batch-initialcasesfilename"].as<std::string>();
        put_flog(LOG_INFO, "got batch initial cases filename %s", g_batchInitialCasesFilename.c_str());
    }

    if(vm.count("batch-parametersfilename"))
    {
        g_batchParametersFilename = vm["batch-parametersfilename"].as<std::string>();
        put_flog(LOG_INFO, "got batch parameters filename %s", g_batchParametersFilename.c_str());
    }

    if(vm.count("batch-outputvariable"))
    {
        g_batchOutputVariable = vm["batch-outputvariable"].as<std::string>();
        put_flog(LOG_INFO, "got batch output variable %s", g_batchOutputVariable.c_str());
    }

    if(vm.count("batch-outputfilename"))
    {
        g_batchOutputFilename = vm["batch-outputfilename"].as<std::string>();
        put_flog(LOG_INFO, "got batch output filename %s", g_batchOutputFilename.c_str());
    }

    // end argument parsing

    // get directory of application
    QDir appDirectory = QDir(QCoreApplication::applicationDirPath());

    // and data directory
    QDir dataDirectory = appDirectory;

#ifdef __APPLE__
    dataDirectory.cdUp();
    dataDirectory.cd("Resources");
    dataDirectory.cd("data");
#else // WIN32 or Linux
    dataDirectory.cdUp();
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

#if USE_DISPLAYCLUSTER
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

    bool success = dcStreamSendSVG(g_dcSocket, "svgStream://" + name, imageData.constData(), imageData.size());

    if(success != true)
    {
        put_flog(LOG_ERROR, "error sending SVG stream");
    }

    return success;
}
#endif
