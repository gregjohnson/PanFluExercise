#ifndef EPIDEMIC_CASES_WIDGET_H
#define EPIDEMIC_CASES_WIDGET_H

#include <QtGui>
#include <boost/shared_ptr.hpp>

class EpidemicDataSet;

struct EpidemicCases
{
    int num;
    int nodeId;
    std::vector<int> stratificationValues;
};

class EpidemicCasesWidget : public QGroupBox
{
    public:

        EpidemicCasesWidget(boost::shared_ptr<EpidemicDataSet> dataSet);

        void setNumCases(int num);
        void setNodeId(int nodeId);

        EpidemicCases getCases();

    private:

        // UI elements
        QSpinBox numCasesSpinBox_;
        QComboBox nodeComboBox_;
        std::vector<QComboBox *> stratificationValueComboBoxes_;
};

#endif
