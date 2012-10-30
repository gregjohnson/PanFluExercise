#include "EpidemicInfoWidget.h"
#include "EpidemicDataSet.h"
#include "log.h"

EpidemicInfoWidget::EpidemicInfoWidget(MainWindow * mainWindow)
{
    // defaults
    time_ = 0;

    QVBoxLayout * layout = new QVBoxLayout();
    setLayout(layout);

    // summary
    QWidget * widget = new QWidget();
    QFormLayout * summaryLayout = new QFormLayout();
    widget->setLayout(summaryLayout);
    layout->addWidget(widget);

    numTimesLabel_.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    summaryLayout->addRow(new QLabel("Num timesteps"), &numTimesLabel_);

    numNodesLabel_.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    summaryLayout->addRow(new QLabel("Num counties"), &numNodesLabel_);

    // table view
    tableView_.horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    tableView_.setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_.setSelectionMode(QAbstractItemView::SingleSelection);
    tableView_.setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_.setModel(&model_);
    layout->addWidget(&tableView_);

    // make connections
    connect((QObject *)mainWindow, SIGNAL(dataSetChanged(boost::shared_ptr<EpidemicDataSet>)), this, SLOT(setDataSet(boost::shared_ptr<EpidemicDataSet>)));

    connect((QObject *)mainWindow, SIGNAL(timeChanged(int)), this, SLOT(setTime(int)));
}

void EpidemicInfoWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    dataSet_ = dataSet;

    // update summary info
    if(dataSet_ != NULL)
    {
        numTimesLabel_.setNum(dataSet_->getNumTimes());
        numNodesLabel_.setNum(dataSet_->getNumNodes());
    }
    else
    {
        numTimesLabel_.setNum(0);
        numNodesLabel_.setNum(0);
    }

    // update table
    updateModel();
}

void EpidemicInfoWidget::setTime(int time)
{
    time_ = time;

    // update table
    updateModel();
}

void EpidemicInfoWidget::updateModel()
{
    // if we don't have a data set, clear the view and return
    if(dataSet_ == NULL)
    {
        model_.clear();
        tableView_.update();

        return;
    }

    // clear model
    model_.clear();

    // set headers
    QStringList headers;
    headers << "County" << "Num infected";
    model_.setHorizontalHeaderLabels(headers);

    QStandardItem * parentItem = model_.invisibleRootItem();

    std::vector<int> nodeIds = dataSet_->getNodeIds();

    for(unsigned int i=0; i<nodeIds.size(); i++)
    {
        QList<QStandardItem *> items;

        items << new QStandardItem(QString(dataSet_->getNodeName(nodeIds[i]).c_str()));

        QStandardItem * item = new QStandardItem();
        item->setData(QVariant(dataSet_->getValue("infected", time_, nodeIds[i])), Qt::DisplayRole);
        items << item;

        parentItem->appendRow(items);
    }

    // enable sorting
    tableView_.setSortingEnabled(true);

    // sort by second column, descending
    tableView_.sortByColumn(1, Qt::DescendingOrder);

    // make sure table view udpates
    tableView_.update();
}
