#ifndef CHART_WIDGET_LINE_H
#define CHART_WIDGET_LINE_H

#include <vtkSmartPointer.h>
#include <vtkPlotPoints.h>
#include <vtkTable.h>

enum CHART_WIDGET_LINE_TYPE { LINE, POINTS };

class ChartWidget;

class ChartWidgetLine
{
    public:

        ChartWidgetLine(ChartWidget * parent = NULL, CHART_WIDGET_LINE_TYPE lineType = LINE);
        ~ChartWidgetLine();

        void clear();
        void setColor(float r, float g, float b);
        void setWidth(float w);
        void setLabel(const char * label);
        void addPoint(double x, double y);
        void addPoint(double y);

        void setAxesToBottomRight();

    private:

        ChartWidget * parent_;

        vtkSmartPointer<vtkTable> table_;
        vtkPlotPoints * plot_;
};

#endif
