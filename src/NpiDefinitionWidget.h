#ifndef NPI_DEFINITION_WIDGET_H
#define NPI_DEFINITION_WIDGET_H

#include <QtGui>
#include <vector>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;
class NpiWidget;

class NpiDefinitionWidget : public QScrollArea
{
    Q_OBJECT

    public:

        NpiDefinitionWidget(MainWindow * mainWindow);

    public slots:

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;

        // UI elements
        QVBoxLayout layout_;
        std::vector<NpiWidget *> npiWidgets_;

        void clearWidgets();
        void updateNpis();

    private slots:

        void addNpi();
};

#endif
