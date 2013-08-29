#include "PriorityGroupWidget.h"
#include "Parameters.h"
#include "EpidemicDataSet.h"
#include "log.h"

PriorityGroupWidget::PriorityGroupWidget()
{
    initialize();
}

PriorityGroupWidget::PriorityGroupWidget(boost::shared_ptr<PriorityGroup> priorityGroup)
{
    initialize();

    // set values according to priorityGroup and disable changes
    nameLineEdit_->setText(priorityGroup->getName().c_str());

    std::vector<std::vector<int> > stratificationVectorValues = priorityGroup->getStratificationVectorValues();

    for(unsigned int i=0; i<stratificationVectorValues.size(); i++)
    {
        if(stratificationVectorValues[i].size() == 1 && stratificationVectorValues[i][0] == STRATIFICATIONS_ALL)
        {
            for(unsigned int j=0; j<stratificationVectorCheckBoxes_[i].size(); j++)
            {
                stratificationVectorCheckBoxes_[i][j]->setCheckState(Qt::Checked);
            }
        }
        else
        {
            for(unsigned int j=0; j<stratificationVectorValues[i].size(); j++)
            {
                int checkedIndex = stratificationVectorValues[i][j];

                stratificationVectorCheckBoxes_[i][checkedIndex]->setCheckState(Qt::Checked);
            }
        }
    }

    disable();
}

void PriorityGroupWidget::initialize()
{
    setTitle("Priority Group");

    QVBoxLayout * layout = new QVBoxLayout();
    setLayout(layout);

    QWidget * nameWidget = new QWidget();
    QHBoxLayout * nameHBox = new QHBoxLayout();
    nameWidget->setLayout(nameHBox);

    QLabel * nameLabel = new QLabel("Name");
    nameLineEdit_ = new QLineEdit();

    nameHBox->addWidget(nameLabel);
    nameHBox->addWidget(nameLineEdit_);

    layout->addWidget(nameWidget);

    // get available stratifications
    std::vector<std::string> stratificationNames = EpidemicDataSet::getStratificationNames();
    std::vector<std::vector<std::string> > stratifications = EpidemicDataSet::getStratifications();

    // for each stratification type
    for(unsigned int i=0; i<stratificationNames.size(); i++)
    {
        QGroupBox * groupBox = new QGroupBox();
        groupBox->setTitle(stratificationNames[i].c_str());

        QVBoxLayout * vBox = new QVBoxLayout();
        groupBox->setLayout(vBox);

        // save the checkboxes
        std::vector<QCheckBox *> checkBoxes;

        // for each stratification entry in type
        for(unsigned int j=0; j<stratifications[i].size(); j++)
        {
            QCheckBox * checkBox = new QCheckBox(stratifications[i][j].c_str());
            checkBox->setCheckState(Qt::Unchecked);

            vBox->addWidget(checkBox);

            checkBoxes.push_back(checkBox);
        }

        layout->addWidget(groupBox);

        stratificationVectorCheckBoxes_.push_back(checkBoxes);

        // hide last stratification (vaccination status)
        // todo: could be handled better...
        if(i == stratificationNames.size() - 1)
        {
            groupBox->hide();
        }
    }

    // cancel / save buttons
    cancelSaveButtonsWidget_ = new QWidget();

    QHBoxLayout * hBox = new QHBoxLayout();
    cancelSaveButtonsWidget_->setLayout(hBox);

    QPushButton * cancelButton = new QPushButton("&Cancel");
    hBox->addWidget(cancelButton);

    QPushButton * saveButton = new QPushButton("&Save");
    hBox->addWidget(saveButton);

    // make connections
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));

    layout->addWidget(cancelSaveButtonsWidget_);
}

void PriorityGroupWidget::disable()
{
    cancelSaveButtonsWidget_->deleteLater();

    setEnabled(false);
}

void PriorityGroupWidget::cancel()
{
    deleteLater();
}

void PriorityGroupWidget::save()
{
    // make sure the name isn't empty
    if(nameLineEdit_->text().isEmpty() == true)
    {
        put_flog(LOG_ERROR, "empty name");

        QMessageBox::warning(this, "Error", "You must specify a priority group name.", QMessageBox::Ok, QMessageBox::Ok);

        return;
    }

    // disable the widgets for further modification
    disable();

    // get values from UI
    std::vector<std::vector<int> > stratificationVectorValues;

    for(unsigned int i=0; i<stratificationVectorCheckBoxes_.size(); i++)
    {
        std::vector<int> stratificationValues;

        for(unsigned int j=0; j<stratificationVectorCheckBoxes_[i].size(); j++)
        {
            if(stratificationVectorCheckBoxes_[i][j]->checkState() == Qt::Checked)
            {
                stratificationValues.push_back(j);
            }
        }

        // if none are checked for the given stratification, then assume all are to be used
        // otherwise, the population of the priority group will be zero
        if(stratificationValues.size() == 0)
        {
            put_flog(LOG_WARN, "no stratifications selected for stratification %i, selecting all", i);

            stratificationValues.push_back(STRATIFICATIONS_ALL);

            // check all the related checkboxes
            for(unsigned int j=0; j<stratificationVectorCheckBoxes_[i].size(); j++)
            {
                stratificationVectorCheckBoxes_[i][j]->setCheckState(Qt::Checked);
            }
        }

        stratificationVectorValues.push_back(stratificationValues);
    }

    // create the priority group
    boost::shared_ptr<PriorityGroup> priorityGroup = boost::shared_ptr<PriorityGroup>(new PriorityGroup(nameLineEdit_->text().toStdString(), stratificationVectorValues));

    // add it to the parameters
    g_parameters.addPriorityGroup(priorityGroup);
}
