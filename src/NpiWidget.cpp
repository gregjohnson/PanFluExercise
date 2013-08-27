#include "NpiWidget.h"
#include "Parameters.h"
#include "EpidemicDataSet.h"
#include "log.h"

NpiWidget::NpiWidget()
{
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
