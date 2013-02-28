#include "StockpileMapWidget.h"
#include "StockpileNetwork.h"
#include "Stockpile.h"
#include "MapShape.h"
#include "EpidemicDataSet.h"

void StockpileMapWidget::setDataSet(boost::shared_ptr<EpidemicDataSet> dataSet)
{
    MapWidget::setDataSet(dataSet);

    if(dataSet != NULL)
    {
        stockpileNetwork_ = dataSet->getStockpileNetwork();

        // todo: disconnect previous signals/slots
        if(stockpileNetwork_ != NULL)
        {
            connect(stockpileNetwork_.get(), SIGNAL(changed()), this, SLOT(updateGL()));
        }
    }
    else
    {
        // NULL
        stockpileNetwork_ = boost::shared_ptr<StockpileNetwork>();
    }
}

void StockpileMapWidget::setTime(int time)
{
    MapWidget::setTime(time);

    // do coloring here, since render() won't be called if the map is offscreen (for SVG export)
    if(stockpileNetwork_ != NULL)
    {
        std::vector<boost::shared_ptr<Stockpile> > stockpiles = stockpileNetwork_->getStockpiles();

        // recolor counties according to stockpile values
        for(unsigned int i=0; i<stockpiles.size(); i++)
        {
            int num = stockpiles[i]->getNum(time_);
            std::vector<int> nodeIds = stockpiles[i]->getNodeIds();

            float population = dataSet_->getPopulation(nodeIds);

            for(unsigned int j=0; j<nodeIds.size(); j++)
            {
                int nodeId = nodeIds[j];

                if(counties_.count(nodeId) != 0)
                {
                    // map to color
                    // scaled to 10% population
                    float r, g, b;
                    countiesColorMap_.getColor3(1. - (float)num / (0.10*population), r, g, b);

                    counties_[nodeId]->setColor(r, g, b);
                }
            }
        }
    }

    // force redraw
    update();

    exportSVGToDisplayCluster();
}

void StockpileMapWidget::render()
{
    renderCountyShapes();
}
