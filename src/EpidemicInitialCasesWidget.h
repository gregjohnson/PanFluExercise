#ifndef EPIDEMIC_INITIAL_CASES_WIDGET_H
#define EPIDEMIC_INITIAL_CASES_WIDGET_H

#include <QtGui>
#include <boost/shared_ptr.hpp>

class MainWindow;
class EpidemicDataSet;
class EpidemicCasesWidget;

class EpidemicInitialCasesWidget : public QScrollArea
{
    Q_OBJECT

    public:

        EpidemicInitialCasesWidget(MainWindow * mainWindow);
        ~EpidemicInitialCasesWidget();

        void applyCases();

    public slots:

        void setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet);
        void loadXmlData(const std::string &filename);

    private:

        // data set information
        boost::shared_ptr<EpidemicDataSet> dataSet_;

        // UI elements
        QVBoxLayout layout_;
        std::vector<EpidemicCasesWidget *> casesWidgets_;

    private slots:

        void clearCases();
        void addCases();
};

#endif
