#ifndef STOCKPILE_CONSUMPTION_WIDGET_H
#define STOCKPILE_CONSUMPTION_WIDGET_H

#include <QtGui>

class PriorityGroupSelectionsWidget;

class StockpileConsumptionWidget : public QScrollArea
{
    Q_OBJECT

    public:

        StockpileConsumptionWidget();

    private:

        // UI elements
        QVBoxLayout layout_;
        PriorityGroupSelectionsWidget * antiviralPriorityGroupSelectionsWidget_;
        PriorityGroupSelectionsWidget * vaccinePriorityGroupSelectionsWidget_;
};

#endif
