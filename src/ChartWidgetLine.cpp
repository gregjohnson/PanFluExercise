#include "ChartWidgetLine.h"
#include "ChartWidget.h"
#include "log.h"
#include <vtkFloatArray.h>
#include <vtkColorSeries.h>
#include <vtkPlotStacked.h>
#include <vtkPlotBar.h>
#include <vtkStringArray.h>
#include <vtkPen.h>
#include <vtkDoubleArray.h>
#include <vtkAxis.h>
#include <vtkTextProperty.h>
#include <sstream>

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
    else if(lineType == CWLT_POINTS)
    {
        plot_ = (vtkPlotPoints *)parent_->getChart()->AddPlot(vtkChart::POINTS);

        plot_->SetMarkerStyle(vtkPlotPoints::CIRCLE);
    }
    else if(lineType == STACKED)
    {
        plot_ = (vtkPlotPoints *)parent_->getChart()->AddPlot(vtkChart::STACKED);

        // set color series
        vtkSmartPointer<vtkColorSeries> colorSeries = vtkSmartPointer<vtkColorSeries>::New();
        colorSeries->SetColorScheme(vtkColorSeries::SPECTRUM);
        ((vtkPlotStacked *)plot_)->SetColorSeries(colorSeries);
    }
    else if(lineType == BAR)
    {
        plot_ = (vtkPlotPoints *)parent_->getChart()->AddPlot(vtkChart::BAR);

        // set color series
        vtkSmartPointer<vtkColorSeries> colorSeries = vtkSmartPointer<vtkColorSeries>::New();
        colorSeries->SetColorScheme(vtkColorSeries::SPECTRUM);
        ((vtkPlotStacked *)plot_)->SetColorSeries(colorSeries);
    }
    else
    {
        put_flog(LOG_ERROR, "unknown lineType");
    }

    plot_->SetInputData(table_, 0, 1);

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

void ChartWidgetLine::setLabels(std::vector<std::string> labels)
{
    vtkSmartPointer<vtkStringArray> vtkLabels = vtkSmartPointer<vtkStringArray>::New();

    for(unsigned int i=0; i<labels.size(); i++)
    {
        vtkLabels->InsertNextValue(labels[i]);
    }

    plot_->SetLabels(vtkLabels);
}

void ChartWidgetLine::setBarLabels(std::vector<std::string> labels)
{
    // custom axis labels for bars, starting at i=0, 1, 2, ...
    vtkSmartPointer<vtkDoubleArray> ticks = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkStringArray> vtkLabels = vtkSmartPointer<vtkStringArray>::New();

    for(unsigned int i=0; i<labels.size(); i++)
    {
        ticks->InsertNextValue(i);
        vtkLabels->InsertNextValue(labels[i]);
    }

    parent_->getChart()->GetAxis(vtkAxis::BOTTOM)->SetCustomTickPositions(ticks, vtkLabels);
    parent_->getChart()->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetOrientation(90);
    parent_->getChart()->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetVerticalJustification(VTK_TEXT_TOP);
    parent_->getChart()->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetJustification(VTK_TEXT_RIGHT);
}

void ChartWidgetLine::clearBarLabels()
{
    // back to defaults
    parent_->getChart()->GetAxis(vtkAxis::BOTTOM)->SetCustomTickPositions(NULL, NULL);
    parent_->getChart()->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetOrientation(0);
    parent_->getChart()->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetVerticalJustification(VTK_TEXT_TOP);
    parent_->getChart()->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetJustification(VTK_TEXT_CENTERED);
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

void ChartWidgetLine::addPoints(double x, std::vector<double> ys)
{
    // make sure we have the correct number of columns
    while((int)ys.size() + 1 > table_->GetNumberOfColumns())
    {
        std::ostringstream oss;
        oss << "Y" << table_->GetNumberOfColumns();

        // additional y array
        vtkSmartPointer<vtkFloatArray> arrY = vtkSmartPointer<vtkFloatArray>::New();
        arrY->SetName(oss.str().c_str());
        table_->AddColumn(arrY);

        plot_->SetInputArray(table_->GetNumberOfColumns() - 1, oss.str().c_str());
    }

    int i = table_->InsertNextBlankRow();
    table_->SetValue(i, 0, x);

    for(unsigned int j=0; j<ys.size(); j++)
    {
        table_->SetValue(i, j+1, ys[j]);
    }

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
