#include "ChartWidgetLine.h"
#include "ChartWidget.h"
#include "log.h"
#include <vtkFloatArray.h>

ChartWidgetLine::ChartWidgetLine(ChartWidget * parent, CHART_WIDGET_LINE_TYPE lineType)
{
    parent_ = parent;

    // table to hold point data
    table_ = vtkSmartPointer<vtkTable>::New();

    // x array
    vtkSmartPointer<vtkFloatArray> arrX = vtkSmartPointer<vtkFloatArray>::New();
    arrX->SetName("X");
    table_->AddColumn(arrX);

    // y array
    vtkSmartPointer<vtkFloatArray> arrY = vtkSmartPointer<vtkFloatArray>::New();
    arrY->SetName("Y");
    table_->AddColumn(arrY);

    // add the line plot and set its input
    if(lineType == LINE)
    {
        plot_ = (vtkPlotPoints *)parent_->getChart()->AddPlot(vtkChart::LINE);
    }
    else if(lineType == POINTS)
    {
        plot_ = (vtkPlotPoints *)parent_->getChart()->AddPlot(vtkChart::POINTS);

        plot_->SetMarkerStyle(vtkPlotPoints::CIRCLE);
    }
    else
    {
        put_flog(LOG_ERROR, "unknown lineType");
    }

    plot_->SetInput(table_, 0, 1);

    // set line plot attributes
    plot_->SetColor(0., 0., 0.);
    plot_->SetWidth(1.);
}

ChartWidgetLine::~ChartWidgetLine()
{
    parent_->getChart()->RemovePlotInstance(plot_);
}

void ChartWidgetLine::clear()
{
    table_->SetNumberOfRows(0);
    table_->Modified();

    parent_->update();
}

void ChartWidgetLine::setColor(float r, float g, float b)
{
    plot_->SetColor(r, g, b);
}

void ChartWidgetLine::setWidth(float w)
{
    plot_->SetWidth(w);
}

void ChartWidgetLine::setLabel(const char * label)
{
    plot_->SetLabel(label);
}

void ChartWidgetLine::addPoint(double x, double y)
{
    int i = table_->InsertNextBlankRow();
    table_->SetValue(i, 0, x);
    table_->SetValue(i, 1, y);

    // have to manually mark the table as modified... seems like this should be done automatically for us
    table_->Modified();

    parent_->update();
}

void ChartWidgetLine::addPoint(double y)
{
    int i = table_->InsertNextBlankRow();
    table_->SetValue(i, 0, (double)i);
    table_->SetValue(i, 1, y);

    // have to manually mark the table as modified... seems like this should be done automatically for us
    table_->Modified();

    parent_->update();
}

void ChartWidgetLine::setAxesToBottomRight()
{
    parent_->getChart()->SetPlotCorner(plot_, 1);
}
