#ifndef PRIORITY_GROUP_DEFINITION_WIDGET_H
#define PRIORITY_GROUP_DEFINITION_WIDGET_H

#include <QtGui>
#include <vector>

class PriorityGroupWidget;

class PriorityGroupDefinitionWidget : public QScrollArea
{
    Q_OBJECT

    public:

        PriorityGroupDefinitionWidget();

    private:

        // UI elements
        QVBoxLayout layout_;
        std::vector<PriorityGroupWidget *> priorityGroupWidgets_;

        void clearWidgets();
        void updatePriorityGroups();

    private slots:

        void addPriorityGroup();
};

#endif
