#include "MainWindow.h"
#include "IliMapWidget.h"
#include "EpidemicMapWidget.h"
#include "StockpileMapWidget.h"
#include "EventMonitor.h"
#include "EventMonitorWidget.h"
#include "TimelineWidget.h"
#include "EpidemicSimulation.h"
#include "EpidemicDataSet.h"
#include "ParametersWidget.h"
#include "Parameters.h"
#include "EpidemicInitialCasesWidget.h"
#include "StockpileNetworkWidget.h"
#include "StockpileConsumptionWidget.h"
#include "PriorityGroupDefinitionWidget.h"
#include "NpiDefinitionWidget.h"
#include "EpidemicInfoWidget.h"
#include "EpidemicChartWidget.h"
#include "StockpileChartWidget.h"
#include "models/disease/StochasticSEATIRD.h"
#include "main.h"
#include "log.h"

MainWindow::MainWindow()
{
    // defaults
    time_ = 0;

    // create menus in menu bar
    QMenu * fileMenu = menuBar()->addMenu("&File");

    // create tool bars
    QToolBar * toolbar = addToolBar("toolbar");

    QToolBar * toolbarBottom = new QToolBar("bottom toolbar", this);
    addToolBar(Qt::BottomToolBarArea, toolbarBottom);

    // new simulation action
    QAction * newSimulationAction = new QAction("New Simulation", this);
    newSimulationAction->setStatusTip("New simulation");
    connect(newSimulationAction, SIGNAL(triggered()), this, SLOT(newSimulation()));

    // open data set action
    /*QAction * openDataSetAction = new QAction("Open Data Set", this);
    openDataSetAction->setStatusTip("Open data set");
    connect(openDataSetAction, SIGNAL(triggered()), this, SLOT(openDataSet()));*/

    // new chart action
    QAction * newChartAction = new QAction("New Chart", this);
    newChartAction->setStatusTip("New chart");
    connect(newChartAction, SIGNAL(triggered()), this, SLOT(newChart()));

    QAction * saveEpidemicDataCsvAction = new QAction("Save Epidemic Data (CSV)", this);
    saveEpidemicDataCsvAction->setStatusTip("Save epidemic data (CSV)");
    connect(saveEpidemicDataCsvAction, SIGNAL(triggered()), this, SLOT(saveEpidemicDataCsv()));

    QAction * loadInitialCasesAction = new QAction("Load Initial Cases", this);
    loadInitialCasesAction->setStatusTip("Load initial cases");
    connect(loadInitialCasesAction, SIGNAL(triggered()), this, SLOT(loadInitialCases()));

    QAction * loadParametersAction = new QAction("Load Parameters", this);
    loadParametersAction->setStatusTip("Load parameters");
    connect(loadParametersAction, SIGNAL(triggered()), this, SLOT(loadParameters()));

#if USE_DISPLAYCLUSTER
    // connect to DisplayCluster action
    QAction * connectToDisplayClusterAction = new QAction("Connect to DisplayCluster", this);
    connectToDisplayClusterAction->setStatusTip("Connect to DisplayCluster");
    connect(connectToDisplayClusterAction, SIGNAL(triggered()), this, SLOT(connectToDisplayCluster()));

    // disconnect from DisplayCluster action
    QAction * disconnectFromDisplayClusterAction = new QAction("Disconnect from DisplayCluster", this);
    disconnectFromDisplayClusterAction->setStatusTip("Disconnect from DisplayCluster");
    connect(disconnectFromDisplayClusterAction, SIGNAL(triggered()), this, SLOT(disconnectFromDisplayCluster()));
#endif

    // add actions to menus
    fileMenu->addAction(newSimulationAction);
    // fileMenu->addAction(openDataSetAction);
    fileMenu->addAction(newChartAction);
    fileMenu->addAction(saveEpidemicDataCsvAction);
    fileMenu->addAction(loadInitialCasesAction);
    fileMenu->addAction(loadParametersAction);

#if USE_DISPLAYCLUSTER
    fileMenu->addAction(connectToDisplayClusterAction);
    fileMenu->addAction(disconnectFromDisplayClusterAction);
#endif

    // add actions to toolbar
    toolbar->addAction(newSimulationAction);
    // toolbar->addAction(openDataSetAction);
    toolbar->addAction(newChartAction);

    // make map widgets the main view
    QTabWidget * tabWidget = new QTabWidget();

    IliMapWidget * iliMapWidget = new IliMapWidget();
    tabWidget->addTab(iliMapWidget, "ILI View");

    EpidemicMapWidget * epidemicMapWidget = new EpidemicMapWidget();
    tabWidget->addTab(epidemicMapWidget, "Infected");

    StockpileMapWidget * antiviralsStockpileMapWidget = new StockpileMapWidget();
    antiviralsStockpileMapWidget->setType(STOCKPILE_ANTIVIRALS);
    tabWidget->addTab(antiviralsStockpileMapWidget, "Antivirals Stockpile");

    StockpileMapWidget * vaccinesStockpileMapWidget = new StockpileMapWidget();
    vaccinesStockpileMapWidget->setType(STOCKPILE_VACCINES);
    tabWidget->addTab(vaccinesStockpileMapWidget, "Vaccines Stockpile");

    setCentralWidget(tabWidget);

    // create event monitor and widget
    EventMonitor * eventMonitor = new EventMonitor(this);

    QDockWidget * eventMonitorDockWidget = new QDockWidget("Event Monitor", this);
    eventMonitorDockWidget->setWidget(new EventMonitorWidget(eventMonitor));
    addDockWidget(Qt::TopDockWidgetArea, eventMonitorDockWidget);

    QDockWidget * timelineDockWidget = new QDockWidget("Timeline", this);
    timelineDockWidget->setWidget(new TimelineWidget(this, eventMonitor));
    addDockWidget(Qt::TopDockWidgetArea, timelineDockWidget);

    // tabify event monitor and timeline widgets
    tabifyDockWidget(eventMonitorDockWidget, timelineDockWidget);

    // setup time slider and add it to bottom toolbar with label
    timeSlider_ = new QSlider(Qt::Horizontal, this);
    connect(timeSlider_, SIGNAL(valueChanged(int)), this, SLOT(setTime(int)));
    toolbarBottom->addWidget(new QLabel("Time"));
    toolbarBottom->addWidget(timeSlider_);

    // previous timestep button
    QAction * previousTimestepAction = new QAction(QIcon::fromTheme("media-seek-backward"), "Back", this);
    previousTimestepAction->setStatusTip(tr("Move backward one day"));
    connect(previousTimestepAction, SIGNAL(triggered()), this, SLOT(previousTimestep()));
    toolbarBottom->addAction(previousTimestepAction);

    // play timesteps button
    playTimestepsAction_ = new QAction(QIcon::fromTheme("media-seek-play"), "Run", this);
    playTimestepsAction_->setStatusTip(tr("Run simulation"));
    playTimestepsAction_->setCheckable(true);
    connect(playTimestepsAction_, SIGNAL(toggled(bool)), this, SLOT(playTimesteps(bool)));
    toolbarBottom->addAction(playTimestepsAction_);

    // next timestep button
    QAction * nextTimestepAction = new QAction(QIcon::fromTheme("media-seek-forward"), "Forward", this);
    nextTimestepAction->setStatusTip(tr("Move forward one day"));
    connect(nextTimestepAction, SIGNAL(triggered()), this, SLOT(nextTimestep()));
    toolbarBottom->addAction(nextTimestepAction);

    // parameters dock
    parametersDockWidget_ = new QDockWidget("Parameters", this);
    parametersDockWidget_->setWidget(new ParametersWidget());
    addDockWidget(Qt::LeftDockWidgetArea, parametersDockWidget_);

    // initial cases dock
    initialCasesWidget_ = new EpidemicInitialCasesWidget(this);
    QDockWidget * initialCasesDockWidget = new QDockWidget("Initial Cases", this);
    initialCasesDockWidget->setWidget(initialCasesWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, initialCasesDockWidget);

    // stockpile network dock
    QDockWidget * stockpileNetworkDockWidget = new QDockWidget("Stockpile", this);
    stockpileNetworkDockWidget->setWidget(new StockpileNetworkWidget(this));
    addDockWidget(Qt::LeftDockWidgetArea, stockpileNetworkDockWidget);

    // stockpile consumption dock
    QDockWidget * stockpileConsumptionDockWidget = new QDockWidget("Stockpile Priorities", this);
    stockpileConsumptionDockWidget->setWidget(new StockpileConsumptionWidget());
    addDockWidget(Qt::LeftDockWidgetArea, stockpileConsumptionDockWidget);

    // priority group definition dock
    QDockWidget * priorityGroupDefinitionDockWidget = new QDockWidget("Priority Groups", this);
    priorityGroupDefinitionDockWidget->setWidget(new PriorityGroupDefinitionWidget());
    addDockWidget(Qt::LeftDockWidgetArea, priorityGroupDefinitionDockWidget);

    // NPI definition dock
    QDockWidget * npiDefinitionDockWidget = new QDockWidget("NPI", this);
    npiDefinitionDockWidget->setWidget(new NpiDefinitionWidget(this));
    addDockWidget(Qt::LeftDockWidgetArea, npiDefinitionDockWidget);

    // info dock
    QDockWidget * infoDockWidget = new QDockWidget("Info", this);
    infoDockWidget->setWidget(new EpidemicInfoWidget(this));
    addDockWidget(Qt::LeftDockWidgetArea, infoDockWidget);

    // tabify parameters, initial cases, stockpile network, and info docks
    tabifyDockWidget(parametersDockWidget_, initialCasesDockWidget);
    tabifyDockWidget(parametersDockWidget_, stockpileNetworkDockWidget);
    tabifyDockWidget(parametersDockWidget_, stockpileConsumptionDockWidget);
    tabifyDockWidget(parametersDockWidget_, priorityGroupDefinitionDockWidget);
    tabifyDockWidget(parametersDockWidget_, npiDefinitionDockWidget);
    tabifyDockWidget(parametersDockWidget_, infoDockWidget);

    // chart docks

    // an epidemic chart
    QDockWidget * chartDockWidget = new QDockWidget("Chart", this);
    chartDockWidget->setWidget(new EpidemicChartWidget(this));
    addDockWidget(Qt::BottomDockWidgetArea, chartDockWidget);

    // and a stockpile chart
    chartDockWidget = new QDockWidget("Chart", this);
    chartDockWidget->setWidget(new StockpileChartWidget(this));
    addDockWidget(Qt::BottomDockWidgetArea, chartDockWidget);

    // make other signal / slot connections
    connect(this, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), iliMapWidget, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));
    connect(this, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), epidemicMapWidget, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));
    connect(this, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), antiviralsStockpileMapWidget, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));
    connect(this, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), vaccinesStockpileMapWidget, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect(this, SIGNAL(dataSetChanged()), this, SLOT(resetTimeSlider()));
    connect(this, SIGNAL(numberOfTimestepsChanged()), this, SLOT(resetTimeSlider()));

    connect(this, SIGNAL(timeChanged(int)), iliMapWidget, SLOT(setTime(int)));
    connect(this, SIGNAL(timeChanged(int)), epidemicMapWidget, SLOT(setTime(int)));
    connect(this, SIGNAL(timeChanged(int)), antiviralsStockpileMapWidget, SLOT(setTime(int)));
    connect(this, SIGNAL(timeChanged(int)), vaccinesStockpileMapWidget, SLOT(setTime(int)));

    connect(&playTimestepsTimer_, SIGNAL(timeout()), this, SLOT(playTimesteps()));

    // show the window
    show();

    // batch mode operations
    if(g_batchMode == true)
    {
        put_flog(LOG_INFO, "starting batch mode");

        // activate other map view
        tabWidget->setCurrentIndex(1);

        newSimulation();

        if(g_batchInitialCasesFilename.empty() != true)
        {
            initialCasesWidget_->loadXmlData(g_batchInitialCasesFilename);
        }

        if(g_batchParametersFilename.empty() != true)
        {
            g_parameters.loadXmlData(g_batchParametersFilename);

            // create a new ParametersWidget to reload these values in the UI
            parametersDockWidget_->setWidget(new ParametersWidget());
        }

        // wait for any GUI events to be processed
        // ignore: QCoreApplication::processEvents();

        for(unsigned int i=0; i<g_batchNumTimesteps; i++)
        {
            nextTimestep();

            // wait for any GUI events to be processed
            // ignore: QCoreApplication::processEvents();
        }

        std::string out = dataSet_->getVariableStratified2NodeVsTime(g_batchOutputVariable);

        {
            std::ofstream ofs(g_batchOutputFilename.c_str());
            ofs << out;
        }

        put_flog(LOG_INFO, "done with batch mode");

        exit(0);
    }
}

MainWindow::~MainWindow()
{

}

QSize MainWindow::sizeHint() const
{
    return QSize(1024, 768);
}

void MainWindow::setTime(int time)
{
    // we only want to perform these actions once per time change...
    if(time_ == time)
    {
        return;
    }

    time_ = time;

    // make sure the time slider has the correct value
    if(timeSlider_->value() != time_)
    {
        timeSlider_->setValue(time_);
    }

    emit(timeChanged(time_));
}

bool MainWindow::previousTimestep()
{
    if(dataSet_ != NULL)
    {
        int previousTime = time_ - 1;

        bool timeInBounds = true;

        if(previousTime < 0)
        {
            timeInBounds = false;
        }

        if(timeInBounds == true)
        {
            setTime(previousTime);

            return true;
        }
    }

    return false;
}

void MainWindow::playTimesteps(bool set)
{
    if(set == true)
    {
        if(playTimestepsTimer_.isActive() == true)
        {
            bool success = nextTimestep();

            if(success != true)
            {
                // uncheck the play button
                playTimestepsAction_->setChecked(false);
            }
        }
        else
        {
            // start the timer
            playTimestepsTimer_.start(PLAY_TIMESTEPS_TIMER_DELAY_MILLISECONDS);
        }
    }
    else
    {
        // stop the timer
        playTimestepsTimer_.stop();
    }

    // wait for any GUI events to be processed
    QCoreApplication::processEvents();
}

bool MainWindow::nextTimestep()
{
    if(dataSet_ != NULL)
    {
        int nextTime = time_ + 1;

        bool timeInBounds = true;

        boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

        if(simulation != NULL)
        {
            // the data set is actually a simulation

            if(nextTime >= simulation->getNumTimes())
            {
                // if this is the first time simulated, set the initial cases
                if(nextTime == 1)
                {
                    initialCasesWidget_->applyCases();
                }

                simulation->simulate();

                // since we've changed the number of timesteps
                emit(numberOfTimestepsChanged());
            }
        }
        else
        {
            // just a regular data set
            if(nextTime >= dataSet_->getNumTimes())
            {
                timeInBounds = false;
            }
        }

        if(timeInBounds == true)
        {
            setTime(nextTime);

            return true;
        }
    }
    else
    {
        QMessageBox::warning(this, "Error", "No active simulation. Click 'New Simulation' in the menu to begin.", QMessageBox::Ok, QMessageBox::Ok);
    }

    return false;
}

void MainWindow::newSimulation()
{
    // use StochasticSEATIRD model
    boost::shared_ptr<EpidemicSimulation> simulation(new StochasticSEATIRD());

    dataSet_ = simulation;

    emit(dataSetChanged(dataSet_));
}

void MainWindow::openDataSet()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Data Set", "", "Simulation files (*.nc)");

    if(!filename.isEmpty())
    {
        boost::shared_ptr<EpidemicDataSet> dataSet(new EpidemicDataSet(filename.toStdString().c_str()));

        if(dataSet->isValid() != true)
        {
            QMessageBox::warning(this, "Error", "Could not load data set.", QMessageBox::Ok, QMessageBox::Ok);
        }
        else
        {
            dataSet_ = dataSet;

            emit(dataSetChanged(dataSet_));
        }
    }
}

void MainWindow::newChart()
{
    QDockWidget * chartDockWidget = new QDockWidget("Chart", this);

    EpidemicChartWidget * epidemicChartWidget = new EpidemicChartWidget(this);

    chartDockWidget->setWidget(epidemicChartWidget);

    addDockWidget(Qt::BottomDockWidgetArea, chartDockWidget);

    chartDockWidget->setFloating(true);

    if(dataSet_ != NULL)
    {
        epidemicChartWidget->setDataSet(dataSet_);
        epidemicChartWidget->setTime(time_);
    }
}

void MainWindow::saveEpidemicDataCsv()
{
    if(dataSet_ == NULL)
    {
        put_flog(LOG_INFO, "NULL dataSet");
        QMessageBox::warning(this, "Error", "No active data set or simulation.", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    // get variable name
    std::vector<std::string> variableNames = dataSet_->getVariableNames();

    QStringList items;

    for(unsigned int i=0; i<variableNames.size(); i++)
    {
        items << variableNames[i].c_str();
    }

    bool ok;
    QString item = QInputDialog::getItem(this, "Select variable", "Variable", items, 0, false, &ok);

    std::string varName;

    if(ok && !item.isEmpty())
    {
        varName = item.toStdString();
    }
    else
    {
        return;
    }

    std::string out = dataSet_->getVariableStratified2NodeVsTime(varName);

    if(out.empty() == true)
    {
        put_flog(LOG_ERROR, "empty");
        QMessageBox::warning(this, "Error", "Could not generate output.", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, "Save Epidemic Data", "", "CSV files (*.csv)");

    if(filename.isNull() == true)
    {
        return;
    }

    // create output file
    std::ofstream ofs(filename.toStdString().c_str());

    // write data to file stream
    ofs << out;
}

void MainWindow::loadInitialCases()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load Initial Cases", "", "XML files (*.xml)");

    if(!filename.isEmpty())
    {
        initialCasesWidget_->loadXmlData(filename.toStdString());
    }
}

void MainWindow::loadParameters()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load Parameters", "", "XML files (*.xml)");

    if(!filename.isEmpty())
    {
        g_parameters.loadXmlData(filename.toStdString());

        // create a new ParametersWidget to reload these values in the UI
        parametersDockWidget_->setWidget(new ParametersWidget());
    }
}

void MainWindow::resetTimeSlider()
{
    if(dataSet_ != NULL)
    {
        timeSlider_->setMinimum(0);
        timeSlider_->setMaximum(dataSet_->getNumTimes() - 1);

        if(time_ >= dataSet_->getNumTimes())
        {
            setTime(0);
        }
    }
    else
    {
        timeSlider_->setMinimum(0);
        timeSlider_->setMaximum(0);
    }
}

#if USE_DISPLAYCLUSTER
void MainWindow::connectToDisplayCluster()
{
    bool ok = false;

    QString hostname = QInputDialog::getText(this, "Connect to DisplayCluster", "Hostname:", QLineEdit::Normal, "localhost", &ok);

    if(ok == true && hostname.isEmpty() != true)
    {
        g_dcSocket = dcStreamConnect(hostname.toStdString().c_str());

        if(g_dcSocket == NULL)
        {
            QMessageBox messageBox;
            messageBox.setText("Could not connect to DisplayCluster.");
            messageBox.exec();
        }
    }
}
#endif

#if USE_DISPLAYCLUSTER
void MainWindow::disconnectFromDisplayCluster()
{
    if(g_dcSocket != NULL)
    {
        dcStreamDisconnect(g_dcSocket);
        g_dcSocket = NULL;
    }
}
#endif
