#ifndef PARAMETERS_WIDGET_H
#define PARAMETERS_WIDGET_H

#define WIDGET_MAX_VALUE 999999999.
#define WIDGET_NUM_DECIMALS 6

#include <string>
#include <QtGui>

class ParametersWidget : public QScrollArea
{
    Q_OBJECT

    public:

        ParametersWidget();

    private:

        void constructParameterWidget(std::string label, std::string description, double value, double min, double max, const char * setSlot);

        QVBoxLayout * layout_;
};

#endif
