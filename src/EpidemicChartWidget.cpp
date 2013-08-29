#include "EpidemicChartWidget.h"
#include "EpidemicDataSet.h"
#include "log.h"

EpidemicChartWidget::EpidemicChartWidget(MainWindow * mainWindow)
{
    // defaults
    time_ = 0;
    nodeId_ = NODES_ALL;
    nodeGroupMode_ = false;
    stratifyByIndex_ = -1;
    stratificationValues_ = std::vector<int>(NUM_STRATIFICATION_DIMENSIONS, STRATIFICATIONS_ALL);

    // add toolbar
    QToolBar * toolbar = addToolBar("toolbar");

    // add node choices to toolbar
    toolbar->addWidget(new QLabel("County"));
    toolbar->addWidget(&nodeComboBox_);

    connect(&nodeComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(setNodeChoice(int)));

    toolbar->addWidget(new QLabel("Variable"));
    toolbar->addWidget(&variableComboBox_);

    connect(&variableComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(setVariableChoice(int)));

    // toolbar line break
    addToolBarBreak();
    toolbar = addToolBar("toolbar");

    // add stratify by choices to toolbar
    std::vector<std::string> stratificationNames = EpidemicDataSet::getStratificationNames();

    stratifyByComboBox_.addItem("None", -1);

    for(unsigned int i=0; i<stratificationNames.size(); i++)
    {
        stratifyByComboBox_.addItem(QString(stratificationNames[i].c_str()), i);
    }

    connect(&stratifyByComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(setStratifyByChoice(int)));

    toolbar->addWidget(new QLabel("Stratify by"));
    toolbar->addWidget(&stratifyByComboBox_);

    // toolbar line break
    addToolBarBreak();
    toolbar = addToolBar("toolbar");

    // add stratification choices to toolbar
    toolbar->addWidget(new QLabel("Filter by"));

    std::vector<std::vector<std::string> > stratifications = EpidemicDataSet::getStratifications();

    for(unsigned int i=0; i<stratifications.size(); i++)
    {
        QComboBox * stratificationValueComboBox = new QComboBox(this);

        stratificationValueComboBox->addItem("All", STRATIFICATIONS_ALL);

        for(unsigned int j=0; j<stratifications[i].size(); j++)
        {
            stratificationValueComboBox->addItem(QString(stratifications[i][j].c_str()), j);
        }

        stratificationValueComboBoxes_.push_back(stratificationValueComboBox);

        connect(stratificationValueComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changedStratificationValueChoice()));

        toolbar->addWidget(stratificationValueComboBox);
    }

    setCentralWidget(&chartWidget_);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect((QObject *)mainWindow, SIGNAL(numberOfTimestepsChanged()), this, SLOT(update()));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));
}

void EpidemicChartWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    // keep track of old selections
    int oldNodeId = nodeId_;
    std::string oldGroupName = groupName_;
    std::string oldVariable = variable_;

    dataSet_ = dataSet;

    // refresh node and variable selections
    nodeComboBox_.clear();
    variableComboBox_.clear();

    if(dataSet != NULL)
    {
        // add node entries
        nodeComboBox_.addItem("All", NODES_ALL);

        // groups
        std::vector<std::string> groupNames = dataSet->getGroupNames();

        for(unsigned int i=0; i<groupNames.size(); i++)
        {
            nodeComboBox_.addItem(groupNames[i].c_str(), groupNames[i].c_str());
        }

        // nodes
        std::vector<int> nodeIds = dataSet->getNodeIds();

        for(unsigned int i=0; i<nodeIds.size(); i++)
        {
            nodeComboBox_.addItem(dataSet->getNodeName(nodeIds[i]).c_str(), nodeIds[i]);
        }

        // add variable entries
        std::vector<std::string> variables = dataSet->getVariableNames();

        for(unsigned int i=0; i<variables.size(); i++)
        {
            variableComboBox_.addItem(variables[i].c_str(), variables[i].c_str());
        }
    }

    // try to update new selections to match previous ones
    int newNodeIndex = -1;

    if(nodeGroupMode_ == false)
    {
        newNodeIndex = nodeComboBox_.findData(oldNodeId);
    }
    else
    {
        newNodeIndex = nodeComboBox_.findData(oldGroupName.c_str());
    }

    int newVariableIndex = variableComboBox_.findData(oldVariable.c_str());

    if(newNodeIndex != -1)
    {
        nodeComboBox_.setCurrentIndex(newNodeIndex);
    }

    if(newVariableIndex != -1)
    {
        variableComboBox_.setCurrentIndex(newVariableIndex);
    }

    update();
}

void EpidemicChartWidget::setTime(int time)
{
    time_ = time;

    if(dataSet_ != NULL && timeIndicator_ != NULL)
    {
        // don't do a full update, just update the time indicator line
        timeIndicator_->clear();

        timeIndicator_->addPoint(time_, 0);
        timeIndicator_->addPoint(time_, 999999999.);
    }

#if USE_DISPLAYCLUSTER
	chartWidget_.exportSVGToDisplayCluster();
#endif
}

void EpidemicChartWidget::setNodeId(int nodeId)
{
    nodeId_ = nodeId;

    nodeGroupMode_ = false;

    update();
}

void EpidemicChartWidget::setGroupName(std::string groupName)
{
    groupName_ = groupName;

    nodeGroupMode_ = true;

    update();
}

void EpidemicChartWidget::setVariable(std::string variable)
{
    variable_ = variable;

    update();

    emit(variableChanged(variable));
}

void EpidemicChartWidget::setStratifyByIndex(int index)
{
    stratifyByIndex_ = index;

    update();

    emit(stratifyByIndexChanged(index));
}

void EpidemicChartWidget::setStratificationValues(std::vector<int> stratificationValues)
{
    stratificationValues_ = stratificationValues;

    update();

    emit(stratificationValuesChanged(stratificationValues));
}

void EpidemicChartWidget::update()
{
    // clear current plots
    chartWidget_.clear();

    // set x-axis label
    std::string xAxisLabel("Time (days)");
    chartWidget_.setXAxisLabel(xAxisLabel);

    // set y-axis label
    std::string yAxisLabel("Population");
    chartWidget_.setYAxisLabel(yAxisLabel);

    if(dataSet_ != NULL)
    {
        // set title
        if(nodeGroupMode_ == false)
        {
            if(nodeId_ == NODES_ALL)
            {
                chartWidget_.setTitle("All Counties");
            }
            else
            {
                chartWidget_.setTitle(dataSet_->getNodeName(nodeId_) + std::string(" County"));
            }
        }
        else
        {
            chartWidget_.setTitle(groupName_);
        }

        // add a (0,0) point to fix bounds calculations for straight horizontal plots
        boost::shared_ptr<ChartWidgetLine> line0 = chartWidget_.getLine();
        line0->setLabel("");
        line0->addPoint(0, 0);

        if(stratifyByIndex_ == -1)
        {
            // no stratifications

            // if we're showing all nodes, go ahead and stratify by group
            if(nodeId_ == NODES_ALL && nodeGroupMode_ == false)
            {
                // add by group
                std::vector<std::string> groupNames = dataSet_->getGroupNames();

                // plot the variable
                boost::shared_ptr<ChartWidgetLine> line = chartWidget_.getLine(NEW_LINE, STACKED);

                line->setWidth(2.);

                std::vector<std::string> labels;

                for(unsigned int i=0; i<groupNames.size(); i++)
                {
                    labels.push_back(variable_ + " (" + groupNames[i] + ")");
                }

                line->setLabels(labels);

                for(int t=0; t<dataSet_->getNumTimes(); t++)
                {
                    std::vector<double> variableValues;

                    for(unsigned int i=0; i<groupNames.size(); i++)
                    {
                        variableValues.push_back(dataSet_->getValue(variable_, t, groupNames[i]));
                    }

                    line->addPoints(t, variableValues);
                }
            }
            else
            {
                // plot the variable
                boost::shared_ptr<ChartWidgetLine> line = chartWidget_.getLine();

                line->setColor(1.,0.,0.);
                line->setWidth(2.);
                line->setLabel(variable_.c_str());

                for(int t=0; t<dataSet_->getNumTimes(); t++)
                {
                    if(nodeGroupMode_ == false)
                    {
                        line->addPoint(t, dataSet_->getValue(variable_, t, nodeId_, stratificationValues_));
                    }
                    else
                    {
                        line->addPoint(t, dataSet_->getValue(variable_, t, groupName_, stratificationValues_));
                    }
                }
            }
        }
        else if(stratifyByIndex_ != -1)
        {
            // add with stratifications

            std::vector<std::vector<std::string> > stratifications = EpidemicDataSet::getStratifications();

            // plot the variable
            boost::shared_ptr<ChartWidgetLine> line = chartWidget_.getLine(NEW_LINE, STACKED);

            line->setWidth(2.);

            std::vector<std::string> labels;

            for(unsigned int i=0; i<stratifications[stratifyByIndex_].size(); i++)
            {
                labels.push_back(variable_ + " (" + stratifications[stratifyByIndex_][i] + ")");
            }

            line->setLabels(labels);

            for(int t=0; t<dataSet_->getNumTimes(); t++)
            {
                std::vector<double> variableValues;

                for(unsigned int i=0; i<stratifications[stratifyByIndex_].size(); i++)
                {
                    std::vector<int> stratificationValues = stratificationValues_;

                    stratificationValues[stratifyByIndex_] = i;

                    if(nodeGroupMode_ == false)
                    {
                        variableValues.push_back(dataSet_->getValue(variable_, t, nodeId_, stratificationValues));
                    }
                    else
                    {
                        variableValues.push_back(dataSet_->getValue(variable_, t, groupName_, stratificationValues));
                    }
                }

                line->addPoints(t, variableValues);
            }
        }

        // clear time indicator
        timeIndicator_ = chartWidget_.getLine();
        timeIndicator_->setWidth(2.);
        timeIndicator_->setLabel("");

        // reset chart bounds
        chartWidget_.resetBounds();
    }
}

void EpidemicChartWidget::setNodeChoice(int choiceIndex)
{
    QVariant::Type type = nodeComboBox_.itemData(choiceIndex).type();

    if(type == QMetaType::Int)
    {
        int index = nodeComboBox_.itemData(choiceIndex).toInt();

        setNodeId(index);
    }
    else if(type == QMetaType::QString)
    {
        std::string groupName = nodeComboBox_.itemData(choiceIndex).toString().toStdString();

        setGroupName(groupName);
    }
    else
    {
        put_flog(LOG_ERROR, "unknown variant type for choice");
        return;
    }
}

void EpidemicChartWidget::setVariableChoice(int choiceIndex)
{
    std::string variable = variableComboBox_.itemData(choiceIndex).toString().toStdString();

    if(variable == "ILI reports")
    {
        // no stratifications,etc. allowed
        stratifyByComboBox_.setCurrentIndex(0);
        stratifyByComboBox_.setEnabled(false);

        for(unsigned int i=0; i<stratificationValueComboBoxes_.size(); i++)
        {
            stratificationValueComboBoxes_[i]->setCurrentIndex(0);
            stratificationValueComboBoxes_[i]->setEnabled(false);
        }
    }
    else
    {
        // make sure widgets are enabled
        stratifyByComboBox_.setEnabled(true);

        for(unsigned int i=0; i<stratificationValueComboBoxes_.size(); i++)
        {
            stratificationValueComboBoxes_[i]->setEnabled(true);
        }
    }

    setVariable(variable);
}

void EpidemicChartWidget::setStratifyByChoice(int choiceIndex)
{
    int index = stratifyByComboBox_.itemData(choiceIndex).toInt();

    setStratifyByIndex(index);
}

void EpidemicChartWidget::changedStratificationValueChoice()
{
    std::vector<int> stratificationValues;

    for(unsigned int i=0; i<stratificationValueComboBoxes_.size(); i++)
    {
        stratificationValues.push_back(stratificationValueComboBoxes_[i]->itemData(stratificationValueComboBoxes_[i]->currentIndex()).toInt());
    }

    setStratificationValues(stratificationValues);
}
