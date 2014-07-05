#ifndef CHART_WIDGET_H
#define CHART_WIDGET_H

#include "ChartWidgetLine.h"
#include <QtGui>
#include <QVTKWidget.h>
#include <vtkContextView.h>
#include <vtkChartXY.h>
#include <vtkGL2PSExporter.h>
#include <QTemporaryFile>
#include <vector>
#include <boost/shared_ptr.hpp>

#define NEW_LINE -1

class ChartWidget : public QVTKWidget
{
    public:

        ChartWidget();
        ~ChartWidget();

        QSize sizeHint() const;

        vtkSmartPointer<vtkChartXY> getChart();

        void setTitle(std::string title);

        void setXAxisLabel(std::string label);
        void setYAxisLabel(std::string label);
        void setRightYAxisLabel(std::string label);

        void resetBounds();

        boost::shared_ptr<ChartWidgetLine> getLine(int index = NEW_LINE, CHART_WIDGET_LINE_TYPE lineType = LINE);
        void clear();

        void setSVGExportAspectRatio(float value);
        void exportSVGToDisplayCluster();

    private:

        // indexing used to identify this chart
        static int numChartWidgets_;
        int index_;

        // VTK requires we use the vtkSmartPointers for these...
        vtkSmartPointer<vtkContextView> view_;
        vtkSmartPointer<vtkChartXY> chart_;

        std::vector<boost::shared_ptr<ChartWidgetLine> > lines_;

        // SVG export
        vtkSmartPointer<vtkGL2PSExporter> svgExporter_;
        vtkSmartPointer<vtkRenderWindow> svgRenderWindow_;
        QTemporaryFile svgTmpFile_;
};

#endif
