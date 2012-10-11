#ifndef CHART_WIDGET_H
#define CHART_WIDGET_H

#include "ChartWidgetLine.h"
#include <QtGui>
#include <QVTKWidget.h>
#include <vtkContextView.h>
#include <vtkChartXY.h>
#include <vector>
#include <boost/shared_ptr.hpp>

class ChartWidget : public QVTKWidget
{
    public:

        ChartWidget();

        QSize sizeHint() const;

        vtkSmartPointer<vtkChartXY> getChart();

        void setTitle(std::string title);

        void setXAxisLabel(std::string label);
        void setYAxisLabel(std::string label);
        void setRightYAxisLabel(std::string label);

        void resetBounds();

        boost::shared_ptr<ChartWidgetLine> getLine(unsigned int index, CHART_WIDGET_LINE_TYPE lineType = LINE);
        void clear();

    private:

        // VTK requires we use the vtkSmartPointers for these...
        vtkSmartPointer<vtkContextView> view_;
        vtkSmartPointer<vtkChartXY> chart_;

        std::vector<boost::shared_ptr<ChartWidgetLine> > lines_;
};

#endif
