#include "MainWindow.h"
#include "MapWidget.h"

MainWindow::MainWindow()
{
    setCentralWidget(new MapWidget());

    // show the window
    show();
}

QSize MainWindow::sizeHint() const
{
    return QSize(1024, 768);
}
