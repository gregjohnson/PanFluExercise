#include "NpiDefinitionWidget.h"
#include "NpiWidget.h"
#include "MainWindow.h"
#include "EpidemicSimulation.h"
#include "Parameters.h"
#include "log.h"

NpiDefinitionWidget::NpiDefinitionWidget(MainWindow * mainWindow)
{
    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    // add NPI button
    QPushButton * addNpiButton = new QPushButton("&Add NPI");
    layout_.addWidget(addNpiButton);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect(addNpiButton, SIGNAL(clicked()), this, SLOT(addNpi()));
}

void NpiDefinitionWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    clearWidgets();
}

void NpiDefinitionWidget::clearWidgets()
{
    // also, clear them from Parameters
    // TODO: this may better be handled via separate signal-slot connections...
    g_parameters.clearNpis();

    // delete all existing widgets
    for(unsigned int i=0; i<npiWidgets_.size(); i++)
    {
        delete npiWidgets_[i];
    }

    npiWidgets_.clear();
}

void NpiDefinitionWidget::addNpi()
{
    boost::shared_ptr<EpidemicSimulation> simulation = boost::dynamic_pointer_cast<EpidemicSimulation>(dataSet_);

    if(simulation != NULL)
    {
        NpiWidget * npiWidget = new NpiWidget(simulation);

        npiWidgets_.push_back(npiWidget);
        layout_.insertWidget(1, npiWidget);
    }
    else
    {
        put_flog(LOG_ERROR, "not a valid simulation");

        QMessageBox::warning(this, "Error", "Not a valid simulation.", QMessageBox::Ok, QMessageBox::Ok);
    }
}
