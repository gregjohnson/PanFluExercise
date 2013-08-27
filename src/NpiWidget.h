#ifndef NPI_WIDGET_H
#define NPI_WIDGET_H

#include "Npi.h"
#include <boost/shared_ptr.hpp>
#include <QtGui>

class NpiWidget : public QGroupBox
{
    Q_OBJECT

    public:

        NpiWidget();

    private:

        QLineEdit * nameLineEdit_;

        QWidget * cancelSaveButtonsWidget_;

        void initialize();
        void disable();

    private slots:

        void cancel();
        void save();
};

#endif
