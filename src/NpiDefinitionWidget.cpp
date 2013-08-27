#include "NpiDefinitionWidget.h"
#include "NpiWidget.h"
#include "log.h"

NpiDefinitionWidget::NpiDefinitionWidget()
{
    QWidget * widget = new QWidget();
    widget->setLayout(&layout_);
    setWidgetResizable(true);
    setWidget(widget);

    // add NPI button
    QPushButton * addNpiButton = new QPushButton("&Add Intervention");
    layout_.addWidget(addNpiButton);

    // make connections
    connect(addNpiButton, SIGNAL(clicked()), this, SLOT(addNpi()));
}

void NpiDefinitionWidget::clearWidgets()
{
    // delete all existing widgets
    for(unsigned int i=0; i<npiWidgets_.size(); i++)
    {
        delete npiWidgets_[i];
    }

    npiWidgets_.clear();
}

void NpiDefinitionWidget::addNpi()
{
    NpiWidget * npiWidget = new NpiWidget();

    npiWidgets_.push_back(npiWidget);
    layout_.insertWidget(1, npiWidget);
}
