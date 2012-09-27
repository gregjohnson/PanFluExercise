#include "MainWindow.h"
#include "MapWidget.h"
#include "EpidemicDataSet.h"

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

    // open content action
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

    // setup time slider and add it to bottom toolbar
    timeSlider_ = new QSlider(Qt::Horizontal, this);
    connect(timeSlider_, SIGNAL(valueChanged(int)), this, SLOT(setTime(int)));
    toolbarBottom->addWidget(timeSlider_);

    // chart dock
    QDockWidget * chartDockWidget = new QDockWidget("Chart", this);
    chartDockWidget->setWidget(&chartWidget_);
    addDockWidget(Qt::BottomDockWidgetArea, chartDockWidget);

    // make other signal / slot connections
    connect(this, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), mapWidget_, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect(this, SIGNAL(dataSetChanged()), this, SLOT(updateChartWidget()));

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

void MainWindow::updateChartWidget()
{
    // clear current plots
    chartWidget_.clear();

    // set x-axis label
    std::string xAxisLabel("Time (days)");
    chartWidget_.setXAxisLabel(xAxisLabel);

    // set y-axis label
    std::string yAxisLabel("Population");
    chartWidget_.setYAxisLabel(yAxisLabel);

    if(dataSet_ != NULL)
    {
        std::vector<std::string> variableNames; // = dataSet_->getVariableNames();

        variableNames.push_back("infected");

        for(unsigned int i=0; i<variableNames.size(); i++)
        {
            // plot the variable
            chartWidget_.getLine(i)->setColor(1.,0.,0.);
            chartWidget_.getLine(i)->setWidth(2.);
            chartWidget_.getLine(i)->setLabel(variableNames[i].c_str());

            for(int t=0; t<dataSet_->getNumTimes(); t++)
            {
                chartWidget_.getLine(i)->addPoint(t, dataSet_->getValue(variableNames[i], t, NODES_ALL));
            }
        }
    }

    chartWidget_.resetBounds();
}
