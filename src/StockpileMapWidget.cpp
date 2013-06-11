#include "StockpileMapWidget.h"
#include "StockpileNetwork.h"
#include "MapShape.h"
#include "EpidemicDataSet.h"

StockpileMapWidget::StockpileMapWidget()
{
    // defaults
    type_ = (STOCKPILE_TYPE)0;

    std::vector<Color> colorVector;

    colorVector.push_back(Color(.60, .847, .788));
    colorVector.push_back(Color(.137, .545, .271));
    colorVector.push_back(Color(0., .267, .106));

    countiesColorMap_.setColorVector(colorVector);

    setTitle(std::string("Available ") + Stockpile::getTypeName(type_) + std::string(" by County as a Fraction of Population"));
    setColorMapMinLabel("0%");
    setColorMapMaxLabel("1%");
}

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
                int num = stockpile->getNum(time_, type_);

                float population = dataSet_->getPopulation(nodeIds[i]);

                if(counties_.count(nodeIds[i]) != 0)
                {
                    // map to color
                    // scaled to 1% population
                    float r, g, b;
                    countiesColorMap_.getColor3((float)num / (0.01*population), r, g, b);

                    counties_[nodeIds[i]]->setColor(r, g, b);
                }
            }
        }
    }

    // force redraw
    update();
	
    exportSVGToDisplayCluster();
}

void StockpileMapWidget::setType(STOCKPILE_TYPE type)
{
    type_ = type;

    update();
}

void StockpileMapWidget::render(QPainter* painter)
{
    renderCountyShapes(painter);
}
