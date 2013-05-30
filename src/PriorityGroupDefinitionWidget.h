#ifndef PRIORITY_GROUP_DEFINITION_WIDGET_H
#define PRIORITY_GROUP_DEFINITION_WIDGET_H

#include <QtGui>
#include <vector>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;
class PriorityGroupWidget;

class PriorityGroupDefinitionWidget : public QScrollArea
{
    Q_OBJECT

    public:

        PriorityGroupDefinitionWidget(MainWindow * mainWindow);

    public slots:

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        void setTime(int time);

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;

        // UI elements
        QVBoxLayout layout_;
        std::vector<PriorityGroupWidget *> priorityGroupWidgets_;

        void clearWidgets();

    private slots:

        void addPriorityGroup();
};

#endif
