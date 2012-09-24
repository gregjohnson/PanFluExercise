#include "MainWindow.h"

MainWindow::MainWindow()
{
    // show the window
    show();
}

QSize MainWindow::sizeHint() const
{
    return QSize(1024, 768);
}
