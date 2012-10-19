#include "ChartWidget.h"
#include "log.h"
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkContextScene.h>
#include <vtkContextMouseEvent.h>
#include <vtkAxis.h>

ChartWidget::ChartWidget()
{
    // setup the view
    view_ = vtkSmartPointer<vtkContextView>::New();
    view_->GetRenderer()->SetBackground(1.,1.,1.);
    view_->SetInteractor(GetInteractor());
    SetRenderWindow(view_->GetRenderWindow());

    // create chart and add it to view
    chart_ = vtkSmartPointer<vtkChartXY>::New();
    view_->GetScene()->AddItem(chart_);

    // show legend
    chart_->SetShowLegend(true);

    // don't allow zooming out beyond range of data
    chart_->SetForceAxesToBounds(true);

    // right button zooms (instead of selects)
    chart_->SetActionToButton(vtkChart::ZOOM, vtkContextMouseEvent::RIGHT_BUTTON);
}

QSize ChartWidget::sizeHint() const
{
    return QSize(400, 300);
}

vtkSmartPointer<vtkChartXY> ChartWidget::getChart()
{
    return chart_;
}

void ChartWidget::setTitle(std::string title)
{
    chart_->SetTitle(title);
}

void ChartWidget::setXAxisLabel(std::string label)
{
    chart_->GetAxis(vtkAxis::BOTTOM)->SetTitle(label);
}

void ChartWidget::setYAxisLabel(std::string label)
{
    chart_->GetAxis(vtkAxis::LEFT)->SetTitle(label);
}

void ChartWidget::setRightYAxisLabel(std::string label)
{
    chart_->GetAxis(vtkAxis::RIGHT)->SetTitle(label);
}

void ChartWidget::resetBounds()
{
    chart_->RecalculateBounds();

    // need to do a render for new bounds take effect
    view_->GetRenderWindow()->Render();
}

boost::shared_ptr<ChartWidgetLine> ChartWidget::getLine(int index, CHART_WIDGET_LINE_TYPE lineType)
{
    if(index == NEW_LINE || index == (int)lines_.size())
    {
        boost::shared_ptr<ChartWidgetLine> line(new ChartWidgetLine(this, lineType));
        lines_.push_back(line);

        return line;
    }
    else if(index > (int)lines_.size())
    {
        put_flog(LOG_ERROR, "cannot create new line with index %i", index);
        return boost::shared_ptr<ChartWidgetLine>();
    }

    return lines_[index];
}

void ChartWidget::clear()
{
    lines_.clear();
}
