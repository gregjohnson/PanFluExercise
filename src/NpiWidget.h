#ifndef NPI_WIDGET_H
#define NPI_WIDGET_H

#include "Npi.h"
#include <boost/shared_ptr.hpp>
#include <QtGui>

class EpidemicDataSet;

class NpiWidget : public QGroupBox
{
    Q_OBJECT

    public:

        NpiWidget(boost::shared_ptr<EpidemicDataSet> dataSet);

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;

        // UI elements
        QLineEdit * nameLineEdit_;

        QLabel executionTimeLabel_;

        QSpinBox * durationSpinBox_;

        std::vector<QDoubleSpinBox *> ageEffectivenessSpinBoxes_;

        QComboBox locationTypeComboBox_;

        QGroupBox * groupGroupBox_;
        std::vector<QCheckBox *> groupCheckBoxes_;

        QGroupBox * nodeGroupBox_;
        std::vector<QCheckBox *> nodeCheckBoxes_;

        QWidget * cancelSaveButtonsWidget_;

        void initialize();
        void disable();

    private slots:

        void setLocationType(int index);

        void cancel();
        void save();
};

#endif
