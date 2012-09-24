#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtGui>

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:

        MainWindow();

        QSize sizeHint() const;
};

#endif
