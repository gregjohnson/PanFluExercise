#include "IliMapWidget.h"
#include "EpidemicDataSet.h"
#include "models/disease/StochasticSEATIRD.h"
#include "MapShape.h"

IliMapWidget::IliMapWidget()
{
    setTitle("ILI Reported Cases by County");
    setColorMapMinLabel("0%");
    setColorMapMaxLabel("1%");
}

void IliMapWidget::setTime(int time)
{
    MapWidget::setTime(time);

    // recolor counties
    if(dataSet_ != NULL)
    {
        std::map<int, boost::shared_ptr<MapShape> >::iterator iter;

        for(iter=counties_.begin(); iter!=counties_.end(); iter++)
        {
            // render grayed out if county has no providers
            bool hasProvider = true;

            boost::shared_ptr<StochasticSEATIRD> simulation = boost::dynamic_pointer_cast<StochasticSEATIRD>(dataSet_);

            if(simulation != NULL)
            {
                std::vector<Provider> providers = simulation->getIliProviders();

                if(providers[simulation->getNodeIndex(iter->first)].status.size() == 0)
                {
                    hasProvider = false;
                }
            }

            if(hasProvider == true)
            {
                // get ILI
                float iliFraction = dataSet_->getValue("ILI", time_, iter->first) / dataSet_->getPopulation(iter->first);

                // map to color
                float r, g, b;
                countiesColorMap_.getColor3(iliFraction / 0.01, r, g, b);

                iter->second->setColor(r, g, b);
            }
            else
            {
                // no providers
                float r, g, b;
                r = g = b = 0.25;

                iter->second->setColor(r, g, b);
            }
        }
    }

    // force redraw
    update();

#if USE_DISPLAYCLUSTER
    exportSVGToDisplayCluster();
#endif
}

void IliMapWidget::render(QPainter * painter)
{
    renderCountyShapes(painter);
}
