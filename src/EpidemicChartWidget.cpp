#include "EpidemicChartWidget.h"
#include "EpidemicDataSet.h"

EpidemicChartWidget::EpidemicChartWidget(MainWindow * mainWindow)
{
    // defaults
    time_ = 0;
    nodeId_ = NODES_ALL;

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));
}

void EpidemicChartWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    update();
}

void EpidemicChartWidget::setTime(int time)
{
    time_ = time;

    if(dataSet_ != NULL && timeIndicator_ != NULL)
    {
        // don't do a full update, just update the time indicator line
        timeIndicator_->clear();

        timeIndicator_->setWidth(2.);

        timeIndicator_->addPoint(time_, 0);
        timeIndicator_->addPoint(time_, 999999999.);

        timeIndicator_->setLabel("");
    }
}

void EpidemicChartWidget::setNodeId(int nodeId)
{
    nodeId_ = nodeId;

    update();
}

void EpidemicChartWidget::update()
{
    // clear current plots
    clear();

    // set x-axis label
    std::string xAxisLabel("Time (days)");
    setXAxisLabel(xAxisLabel);

    // set y-axis label
    std::string yAxisLabel("Population");
    setYAxisLabel(yAxisLabel);

    if(dataSet_ != NULL)
    {
        // set title
        if(nodeId_ == NODES_ALL)
        {
            setTitle("All Counties");
        }
        else
        {
            setTitle(dataSet_->getNodeName(nodeId_) + std::string(" County"));
        }

        std::vector<std::string> variableNames;

        variableNames.push_back("infected");

        for(unsigned int i=0; i<variableNames.size(); i++)
        {
            // plot the variable
            getLine(i)->setColor(1.,0.,0.);
            getLine(i)->setWidth(2.);
            getLine(i)->setLabel(variableNames[i].c_str());

            for(int t=0; t<dataSet_->getNumTimes(); t++)
            {
                getLine(i)->addPoint(t, dataSet_->getValue(variableNames[i], t, nodeId_));
            }
        }

        // add by group if we're plotting all nodes
        if(nodeId_ == NODES_ALL)
        {
            std::vector<std::string> groupNames = dataSet_->getGroupNames();

            for(unsigned int i=0; i<groupNames.size(); i++)
            {
                for(unsigned int j=0; j<variableNames.size(); j++)
                {
                    int lineIndex = variableNames.size() + i*variableNames.size() + j;

                    QColor color = QColor::fromHsvF((float)i / (float)groupNames.size(), 1., 1.);
                    std::string label = variableNames[j] + " (" + groupNames[i] + ")";

                    getLine(lineIndex)->setColor(color.redF(), color.greenF(), color.blueF());
                    getLine(lineIndex)->setWidth(1.);
                    getLine(lineIndex)->setLabel(label.c_str());

                    for(int t=0; t<dataSet_->getNumTimes(); t++)
                    {
                        getLine(lineIndex)->addPoint(t, dataSet_->getValue(variableNames[j], t, groupNames[i]));
                    }
                }
            }
        }

        resetBounds();

        // keep a line for the time indicator
        timeIndicator_ = getLine();
    }
}
