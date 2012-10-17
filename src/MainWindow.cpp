#include "MainWindow.h"
#include "MapWidget.h"
#include "EpidemicDataSet.h"
#include "EpidemicInfoWidget.h"
#include "EpidemicChartWidget.h"

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

    // open data set action
    QAction * openDataSetAction = new QAction("Open Data Set", this);
    openDataSetAction->setStatusTip("Open data set");
    connect(openDataSetAction, SIGNAL(triggered()), this, SLOT(openDataSet()));

    // add actions to menus
    fileMenu->addAction(openDataSetAction);

    // add actions to toolbar
    toolbar->addAction(openDataSetAction);

    // make a map widget the main view
    mapWidget_ = new MapWidget();
    setCentralWidget(mapWidget_);

    // setup time slider and add it to bottom toolbar with label
    timeSlider_ = new QSlider(Qt::Horizontal, this);
    connect(timeSlider_, SIGNAL(valueChanged(int)), this, SLOT(setTime(int)));
    toolbarBottom->addWidget(new QLabel("Time"));
    toolbarBottom->addWidget(timeSlider_);

    // info dock
    QDockWidget * infoDockWidget = new QDockWidget("Info", this);
    infoDockWidget->setWidget(new EpidemicInfoWidget(this));
    addDockWidget(Qt::LeftDockWidgetArea, infoDockWidget);

    // chart dock
    QDockWidget * chartDockWidget = new QDockWidget("Chart", this);
    chartDockWidget->setWidget(new EpidemicChartWidget(this));
    addDockWidget(Qt::BottomDockWidgetArea, chartDockWidget);

    // chart docks for specific counties

    // Travis
    chartDockWidget = new QDockWidget("Chart", this);
    EpidemicChartWidget * epidemicChartWidget = new EpidemicChartWidget(this);
    epidemicChartWidget->setNodeId(453);
    chartDockWidget->setWidget(epidemicChartWidget);
    addDockWidget(Qt::RightDockWidgetArea, chartDockWidget);

    // Harris
    chartDockWidget = new QDockWidget("Chart", this);
    epidemicChartWidget = new EpidemicChartWidget(this);
    epidemicChartWidget->setNodeId(201);
    chartDockWidget->setWidget(epidemicChartWidget);
    addDockWidget(Qt::RightDockWidgetArea, chartDockWidget);

    // El Paso
    chartDockWidget = new QDockWidget("Chart", this);
    epidemicChartWidget = new EpidemicChartWidget(this);
    epidemicChartWidget->setNodeId(141);
    chartDockWidget->setWidget(epidemicChartWidget);
    addDockWidget(Qt::RightDockWidgetArea, chartDockWidget);

    // make other signal / slot connections
    connect(this, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), mapWidget_, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect(this, SIGNAL(dataSetChanged()), this, SLOT(resetTimeSlider()));

    connect(this, SIGNAL(timeChanged(int)), mapWidget_, SLOT(setTime(int)));

    // show the window
    show();
}

MainWindow::~MainWindow()
{
    delete mapWidget_;
}

QSize MainWindow::sizeHint() const
{
    return QSize(1024, 768);
}

void MainWindow::setTime(int time)
{
    time_ = time;

    emit(timeChanged(time_));
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

void MainWindow::resetTimeSlider()
{
    if(dataSet_ != NULL)
    {
        timeSlider_->setMinimum(0);
        timeSlider_->setMaximum(dataSet_->getNumTimes() - 1);

        setTime(0);
    }
    else
    {
        timeSlider_->setMinimum(0);
        timeSlider_->setMaximum(0);
    }
}
