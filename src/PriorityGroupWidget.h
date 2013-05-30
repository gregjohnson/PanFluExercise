#ifndef PRIORITY_GROUP_WIDGET_H
#define PRIORITY_GROUP_WIDGET_H

#include "PriorityGroup.h"
#include <QtGui>
#include <boost/shared_ptr.hpp>

class EpidemicDataSet;

class PriorityGroupWidget : public QGroupBox
{
    Q_OBJECT

    public:

        PriorityGroupWidget(boost::shared_ptr<EpidemicDataSet> dataSet);

        // this constructor creates the widget corresponding to priorityGroup, and disables it for modification
        // it assumes the priority group has already been added to the data set
        PriorityGroupWidget(boost::shared_ptr<EpidemicDataSet> dataSet, PriorityGroup priorityGroup);

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;

        QLineEdit * nameLineEdit_;
        std::vector<std::vector<QCheckBox *> > stratificationVectorCheckBoxes_;

        QWidget * cancelSaveButtonsWidget_;

        void initialize();
        void disable();

    private slots:

        void cancel();
        void save();
};

#endif
