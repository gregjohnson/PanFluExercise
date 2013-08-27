#ifndef NPI_DEFINITION_WIDGET_H
#define NPI_DEFINITION_WIDGET_H

#include <QtGui>
#include <vector>

class NpiWidget;

class NpiDefinitionWidget : public QScrollArea
{
    Q_OBJECT

    public:

        NpiDefinitionWidget();

    private:

        // UI elements
        QVBoxLayout layout_;
        std::vector<NpiWidget *> npiWidgets_;

        void clearWidgets();
        void updateNpis();

    private slots:

        void addNpi();
};

#endif
