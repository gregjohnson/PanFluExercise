#include "EpidemicCasesWidget.h"
#include "EpidemicDataSet.h"

EpidemicCasesWidget::EpidemicCasesWidget(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    setTitle("Cases");

    QVBoxLayout * layout = new QVBoxLayout();
    setLayout(layout);

    // add num cases widget
    numCasesSpinBox_.setMaximum(999999);
    numCasesSpinBox_.setSuffix(" cases");
    layout->addWidget(&numCasesSpinBox_);

    // add node choices widget
    layout->addWidget(&nodeComboBox_);

    std::vector<int> nodeIds = dataSet->getNodeIds();

    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        nodeComboBox_.addItem(dataSet->getNodeName(nodeIds[i]).c_str(), nodeIds[i]);
    }

    // add stratification choices
    std::vector<std::vector<std::string> > stratifications = EpidemicDataSet::getStratifications();

    for(unsigned int i=0; i<stratifications.size(); i++)
    {
        QComboBox * stratificationValueComboBox = new QComboBox(this);

        for(unsigned int j=0; j<stratifications[i].size(); j++)
        {
            stratificationValueComboBox->addItem(QString(stratifications[i][j].c_str()), j);
        }

        stratificationValueComboBoxes_.push_back(stratificationValueComboBox);

        layout->addWidget(stratificationValueComboBox);
    }

    // default to second age group (first stratification)
    stratificationValueComboBoxes_[0]->setCurrentIndex(1);

    // hide last stratification (vaccination status)
    // todo: could be handled better...
    stratificationValueComboBoxes_[stratificationValueComboBoxes_.size()-1]->hide();
}

void EpidemicCasesWidget::setNumCases(int num)
{
    numCasesSpinBox_.setValue(num);
}

void EpidemicCasesWidget::setNodeId(int nodeId)
{
    nodeComboBox_.setCurrentIndex(nodeComboBox_.findData(nodeId));
}

EpidemicCases EpidemicCasesWidget::getCases()
{
    EpidemicCases cases;

    cases.num = numCasesSpinBox_.value();

    cases.nodeId = nodeComboBox_.itemData(nodeComboBox_.currentIndex()).toInt();

    std::vector<int> stratificationValues;

    for(unsigned int i=0; i<stratificationValueComboBoxes_.size(); i++)
    {
        stratificationValues.push_back(stratificationValueComboBoxes_[i]->itemData(stratificationValueComboBoxes_[i]->currentIndex()).toInt());
    }

    cases.stratificationValues = stratificationValues;

    return cases;
}
