#include "PriorityGroupSelectionsWidget.h"
#include "PriorityGroupSelections.h"
#include "PriorityGroup.h"
#include "Parameters.h"
#include "log.h"

// need Stockpile shared_ptr allowed as a QVariant
Q_DECLARE_METATYPE(boost::shared_ptr<PriorityGroup>)

PriorityGroupSelectionsWidget::PriorityGroupSelectionsWidget(std::string title)
{
    initialize(title);
}

boost::shared_ptr<PriorityGroupSelections> PriorityGroupSelectionsWidget::getPriorityGroupSelections()
{
    std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups;

    for(unsigned int i=0; i<priorityGroupCheckBoxes_.size(); i++)
    {
        if(priorityGroupCheckBoxes_[i]->checkState() == Qt::Checked)
        {
            priorityGroups.push_back(priorityGroupCheckBoxes_[i]->property("PriorityGroup").value<boost::shared_ptr<PriorityGroup> >());
        }
    }

    return boost::shared_ptr<PriorityGroupSelections>(new PriorityGroupSelections(priorityGroups));
}

void PriorityGroupSelectionsWidget::addPriorityGroup(boost::shared_ptr<PriorityGroup> priorityGroup)
{
    QCheckBox * checkBox = new QCheckBox(priorityGroup->getName().c_str());
    checkBox->setCheckState(Qt::Unchecked);

    // associate priorityGroup with the check box
    checkBox->setProperty("PriorityGroup", QVariant::fromValue(priorityGroup));

    // make connections
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged()));

    layout_.addWidget(checkBox);

    priorityGroupCheckBoxes_.push_back(checkBox);
}

void PriorityGroupSelectionsWidget::initialize(std::string title)
{
    setTitle(title.c_str());

    setLayout(&layout_);

    // make connections and update available priority groups
    connect(&g_parameters, SIGNAL(priorityGroupAdded(boost::shared_ptr<PriorityGroup>)), this, SLOT(addPriorityGroup(boost::shared_ptr<PriorityGroup>)));

    std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups = g_parameters.getPriorityGroups();

    for(unsigned int i=0; i<priorityGroups.size(); i++)
    {
        addPriorityGroup(priorityGroups[i]);
    }
}

void PriorityGroupSelectionsWidget::disable()
{
    setEnabled(false);
}

void PriorityGroupSelectionsWidget::checkBoxChanged()
{
    boost::shared_ptr<PriorityGroupSelections> priorityGroupSelections = getPriorityGroupSelections();

    emit(priorityGroupSelectionsChanged(priorityGroupSelections));
}
