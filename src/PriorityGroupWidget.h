#ifndef PRIORITY_GROUP_WIDGET_H
#define PRIORITY_GROUP_WIDGET_H

#include "PriorityGroup.h"
#include <boost/shared_ptr.hpp>
#include <QtGui>

class PriorityGroupWidget : public QGroupBox
{
    Q_OBJECT

    public:

        PriorityGroupWidget();

        // this constructor creates the widget corresponding to priorityGroup, and disables it for modification
        // it assumes the priority group has already been added
        PriorityGroupWidget(boost::shared_ptr<PriorityGroup> priorityGroup);

    private:

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
