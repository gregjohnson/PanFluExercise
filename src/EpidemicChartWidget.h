#ifndef EPIDEMIC_CHART_WIDGET_H
#define EPIDEMIC_CHART_WIDGET_H

#include "ChartWidget.h"
#include <QtGui>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;

class EpidemicChartWidget : public QMainWindow
{
    Q_OBJECT

    public:

        EpidemicChartWidget(MainWindow * mainWindow);

        ChartWidget * getChartWidget();

    signals:

        // these signals can be used to connect epidemic charts to each other, for example with the streaming charts
        void variableChanged(std::string variable);
        void stratifyByIndexChanged(int index);
        void stratificationValuesChanged(std::vector<int> stratificationValues);

    public slots:

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        void setTime(int time);
        void setNodeId(int nodeId);
        void setGroupName(std::string groupName);
        void setVariable(std::string variable);

        void setStratifyByIndex(int index);
        void setStratificationValues(std::vector<int> stratificationValues);

        void update();

    private:

        // the chart widget
        ChartWidget chartWidget_;

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;
        int time_;
        int nodeId_;
        std::string groupName_;
        std::string variable_;

        // true if we're doing node groups; false otherwise
        bool nodeGroupMode_;

        // stratification
        int stratifyByIndex_;
        std::vector<int> stratificationValues_;

        // time indicator line
        boost::shared_ptr<ChartWidgetLine> timeIndicator_;

        // UI elements
        QComboBox nodeComboBox_;
        QComboBox variableComboBox_;
        QComboBox stratifyByComboBox_;
        std::vector<QComboBox *> stratificationValueComboBoxes_;

    private slots:

        void setNodeChoice(int choiceIndex);
        void setVariableChoice(int choiceIndex);
        void setStratifyByChoice(int choiceIndex);
        void changedStratificationValueChoice();
};

#endif
