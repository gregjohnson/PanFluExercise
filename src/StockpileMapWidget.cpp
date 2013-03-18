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
        // get nodeIds
        std::vector<int> nodeIds = stockpileNetwork_->getDataSet()->getNodeIds();

        // recolor counties according to node stockpile values
        for(unsigned int i=0; i<nodeIds.size(); i++)
        {
            boost::shared_ptr<Stockpile> stockpile = stockpileNetwork_->getNodeStockpile(nodeIds[i]);

            if(stockpile != NULL)
            {
                int num = stockpile->getNum(time_);

                float population = dataSet_->getPopulation(nodeIds[i]);

                if(counties_.count(nodeIds[i]) != 0)
                {
                    // map to color
                    // scaled to 10% population
                    float r, g, b;
                    countiesColorMap_.getColor3(1. - (float)num / (0.10*population), r, g, b);

                    counties_[nodeIds[i]]->setColor(r, g, b);
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
