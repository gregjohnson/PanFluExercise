#include "NpiWidget.h"
#include "Parameters.h"
#include "EpidemicDataSet.h"
#include "log.h"

NpiWidget::NpiWidget(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    initialize();
}

void NpiWidget::initialize()
{
    setTitle("Intervention");

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

    // add other widgets

    // add location type choices widget
    {
        layout->addWidget(&locationTypeComboBox_);
        locationTypeComboBox_.addItem("Statewide", "statewide");
        locationTypeComboBox_.addItem("By region", "region");
        locationTypeComboBox_.addItem("By county", "county");

        connect(&locationTypeComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(setLocationType(int)));
    }

    // add group checkboxes widgets
    {
        groupGroupBox_ = new QGroupBox();
        groupGroupBox_->setTitle("Regions");

        QVBoxLayout * vBox = new QVBoxLayout();
        groupGroupBox_->setLayout(vBox);

        std::vector<std::string> groupNames = dataSet_->getGroupNames();

        // for each group name
        for(unsigned int i=0; i<groupNames.size(); i++)
        {
            QCheckBox * checkBox = new QCheckBox(groupNames[i].c_str());
            checkBox->setCheckState(Qt::Unchecked);

            vBox->addWidget(checkBox);

            groupCheckBoxes_.push_back(checkBox);
        }

        layout->addWidget(groupGroupBox_);

        // hide by default
        groupGroupBox_->hide();
    }

    // add node checkboxes widgets
    {
        nodeGroupBox_ = new QGroupBox();
        nodeGroupBox_->setTitle("Counties");

        QVBoxLayout * vBox = new QVBoxLayout();
        nodeGroupBox_->setLayout(vBox);

        std::vector<int> nodeIds = dataSet_->getNodeIds();

        // for each node
        for(unsigned int i=0; i<nodeIds.size(); i++)
        {
            QCheckBox * checkBox = new QCheckBox(dataSet_->getNodeName(nodeIds[i]).c_str());
            checkBox->setCheckState(Qt::Unchecked);

            vBox->addWidget(checkBox);

            nodeCheckBoxes_.push_back(checkBox);
        }

        layout->addWidget(nodeGroupBox_);

        // hide by default
        nodeGroupBox_->hide();
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

void NpiWidget::disable()
{
    cancelSaveButtonsWidget_->deleteLater();

    setEnabled(false);
}

void NpiWidget::setLocationType(int index)
{
    if(locationTypeComboBox_.itemData(index) == "statewide")
    {
        groupGroupBox_->hide();
        nodeGroupBox_->hide();
    }
    else if(locationTypeComboBox_.itemData(index) == "region")
    {
        groupGroupBox_->show();
        nodeGroupBox_->hide();
    }
    else if(locationTypeComboBox_.itemData(index) == "county")
    {
        groupGroupBox_->hide();
        nodeGroupBox_->show();
    }
    else
    {
        put_flog(LOG_ERROR, "unknown location type");
        return;
    }
}

void NpiWidget::cancel()
{
    deleteLater();
}

void NpiWidget::save()
{
    // make sure the name isn't empty
    if(nameLineEdit_->text().isEmpty() == true)
    {
        put_flog(LOG_ERROR, "empty name");

        QMessageBox::warning(this, "Error", "You must specify an intervention name.", QMessageBox::Ok, QMessageBox::Ok);

        return;
    }

    // disable the widgets for further modification
    disable();

    // get values from UI

    // create the NPI
    boost::shared_ptr<Npi> npi = boost::shared_ptr<Npi>(new Npi(nameLineEdit_->text().toStdString()));

    // add it to the parameters
    g_parameters.addNpi(npi);
}
