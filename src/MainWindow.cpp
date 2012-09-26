#include "MainWindow.h"
#include "MapWidget.h"
#include "EpidemicDataSet.h"

MainWindow::MainWindow()
{
    // create menus in menu bar
    QMenu * fileMenu = menuBar()->addMenu("&File");

    // create tool bar
    QToolBar * toolbar = addToolBar("toolbar");

    // open content action
    QAction * openDataSetAction = new QAction("Open Data Set", this);
    openDataSetAction->setStatusTip("Open data set");
    connect(openDataSetAction, SIGNAL(triggered()), this, SLOT(openDataSet()));

    // add actions to menus
    fileMenu->addAction(openDataSetAction);

    // add actions to toolbar
    toolbar->addAction(openDataSetAction);

    // make a map widget the main view
    setCentralWidget(new MapWidget());

    // show the window
    show();
}

QSize MainWindow::sizeHint() const
{
    return QSize(1024, 768);
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
        }
    }
}
