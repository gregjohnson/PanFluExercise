#include "ChartWidget.h"
#include "main.h"
#include "log.h"
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkContextScene.h>
#include <vtkContextMouseEvent.h>
#include <vtkAxis.h>
#include <vtkChartLegend.h>

#if USE_DISPLAYCLUSTER
    #include "dcStream.h"
#endif

int ChartWidget::numChartWidgets_ = 0;

ChartWidget::ChartWidget()
{
    // indexing
    index_ = numChartWidgets_;
    numChartWidgets_++;

    // setup the view
    view_ = vtkSmartPointer<vtkContextView>::New();
    view_->GetRenderer()->SetBackground(1.,1.,1.);
    view_->SetInteractor(GetInteractor());
    SetRenderWindow(view_->GetRenderWindow());

    // create chart and add it to view
    chart_ = vtkSmartPointer<vtkChartXY>::New();
    view_->GetScene()->AddItem(chart_);

    // show legend, by default in the lower left
    chart_->SetShowLegend(true);
    chart_->GetLegend()->SetVerticalAlignment(vtkChartLegend::BOTTOM);
    chart_->GetLegend()->SetHorizontalAlignment(vtkChartLegend::LEFT);

    // don't allow zooming out beyond range of data
    chart_->SetForceAxesToBounds(true);

    // right button zooms (instead of selects)
    chart_->SetActionToButton(vtkChart::ZOOM, vtkContextMouseEvent::RIGHT_BUTTON);

    // create SVG exporter
    svgExporter_ = vtkSmartPointer<vtkGL2PSExporter>::New();

    // use an off-screen constant-sized render window for SVG export
    svgRenderWindow_ = vtkSmartPointer<vtkRenderWindow>::New();
    svgRenderWindow_->OffScreenRenderingOn();
    svgRenderWindow_->SetSize(480, 300);

    svgExporter_->SetRenderWindow(svgRenderWindow_);

    svgExporter_->SetFileFormatToSVG();
    svgExporter_->CompressOff();
    svgExporter_->SetSortToOff();
    svgExporter_->TextAsPathOn();
}

ChartWidget::~ChartWidget()
{
    // the tempory file doesn't have the extension as written by the SVG exporter
    // so, we need to delete the file with the extension, too
    if(svgTmpFile_.fileName().isEmpty() != true)
    {
        QFile svgTmpFileOther(svgTmpFile_.fileName() + ".svg");

        if(svgTmpFileOther.exists() == true && svgTmpFileOther.remove() == true)
        {
            put_flog(LOG_DEBUG, "removed temporary file %s", svgTmpFileOther.fileName().toStdString().c_str());
        }
        else
        {
            put_flog(LOG_ERROR, "could not remove temporary file %s", svgTmpFileOther.fileName().toStdString().c_str());
        }
    }
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

#if USE_DISPLAYCLUSTER
void ChartWidget::exportSVGToDisplayCluster()
{
    if(g_dcSocket != NULL && svgTmpFile_.open())
    {
        // move the renderer to the SVG render window
        view_->GetRenderWindow()->RemoveRenderer(view_->GetRenderer());
        svgRenderWindow_->AddRenderer(view_->GetRenderer());

        // write the SVG file
        svgExporter_->SetFilePrefix(svgTmpFile_.fileName().toStdString().c_str());
        svgExporter_->Write();

        put_flog(LOG_DEBUG, "wrote %s", svgTmpFile_.fileName().toStdString().c_str());

        // move the renderer back to the regular render window
        svgRenderWindow_->RemoveRenderer(view_->GetRenderer());
        view_->GetRenderWindow()->AddRenderer(view_->GetRenderer());

        // now, send it to DisplayCluster
        sendSVGToDisplayCluster((svgTmpFile_.fileName() + ".svg").toStdString(), (QString("ExerciseChart-") + QString::number(index_) + ".svg").toStdString());
    }
}
#endif
