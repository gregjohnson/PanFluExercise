#ifndef PRIORITY_GROUP_SELECTIONS_WIDGET_H
#define PRIORITY_GROUP_SELECTIONS_WIDGET_H

#include <string>
#include <boost/shared_ptr.hpp>
#include <QtGui>

class PriorityGroupSelections;
class PriorityGroup;

class PriorityGroupSelectionsWidget : public QGroupBox
{
    Q_OBJECT

    public:

        PriorityGroupSelectionsWidget(std::string title=std::string("Priority Group Selections"));

        // get the current priority group selections based on the checkboxes
        boost::shared_ptr<PriorityGroupSelections> getPriorityGroupSelections();

    signals:

        void priorityGroupSelectionsChanged(boost::shared_ptr<PriorityGroupSelections> priorityGroupSelections);

    public slots:

        void addPriorityGroup(boost::shared_ptr<PriorityGroup> priorityGroup);

    private:

        QVBoxLayout layout_;
        std::vector<QCheckBox *> priorityGroupCheckBoxes_;

        void initialize(std::string title);
        void disable();

    private slots:

        void checkBoxChanged();
};

#endif
