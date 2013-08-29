#include "EpidemicMapWidget.h"
#include "EpidemicDataSet.h"
#include "MapShape.h"

EpidemicMapWidget::EpidemicMapWidget()
{
    setTitle("Infected Population by County");
    setColorMapMinLabel("0%");
    setColorMapMaxLabel("1%");
}

void EpidemicMapWidget::setTime(int time)
{
    MapWidget::setTime(time);

    // recolor counties
    if(dataSet_ != NULL)
    {
        std::map<int, boost::shared_ptr<MapShape> >::iterator iter;

        for(iter=counties_.begin(); iter!=counties_.end(); iter++)
        {
            // get total infectious
            float infectiousFraction = dataSet_->getValue("All infected", time_, iter->first) / dataSet_->getPopulation(iter->first);

            // map to color
            float r, g, b;
            countiesColorMap_.getColor3(infectiousFraction / 0.01, r, g, b);

            iter->second->setColor(r, g, b);
        }
    }

    // force redraw
    update();

#if USE_DISPLAYCLUSTER
    exportSVGToDisplayCluster();
#endif
}

void EpidemicMapWidget::render(QPainter * painter)
{
    renderCountyShapes(painter);
    renderCountyTravel(painter);
}

void EpidemicMapWidget::renderCountyTravel(QPainter * painter)
{
    // parameters
    float infectiousTravelerThreshhold = 1.;
    float infectiousTravelerAlphaScale = 100.;

    if(dataSet_ == NULL)
    {
        return;
    }

    for(std::map<int, boost::shared_ptr<MapShape> >::iterator iter0=counties_.begin(); iter0!=counties_.end(); iter0++)
    {
        int nodeId0 = iter0->first;

        double lat0, lon0;
        iter0->second->getCentroid(lat0, lon0);

        // get number of infectious in node0
        float infectiousNode0 = dataSet_->getValue("infectious", time_, nodeId0);

        if(infectiousNode0 < infectiousTravelerThreshhold)
        {
            continue;
        }

        for(std::map<int, boost::shared_ptr<MapShape> >::iterator iter1=counties_.begin(); iter1!=counties_.end(); iter1++)
        {
            int nodeId1 = iter1->first;

            double lat1, lon1;
            iter1->second->getCentroid(lat1, lon1);

            if(nodeId0 != nodeId1)
            {
                float travel = dataSet_->getTravel(nodeId0, nodeId1);

                float infectiousTravelers = infectiousNode0 * travel;

                if(infectiousTravelers >= infectiousTravelerThreshhold)
                {
                    float alpha = std::min<float>(.5, std::max<float>(.005, infectiousTravelers / infectiousTravelerAlphaScale));

                    painter->setBrush(QBrush(QColor::fromRgbF(1, 0, 0, alpha)));
                    painter->setPen(QPen(QBrush(QColor::fromRgbF(1, 0, 0, alpha * .1)), .1));

                    QVector<QPointF> points;
                    points.push_back(QPointF(lon0, lat0));
                    points.push_back(QPointF(lon1, lat1));

                    QVector2D vec = QVector2D(lon1-lon0, lat1-lat0);
                    vec.normalize();
                    vec *= .006;
                    points.push_back(QPointF(lon1 - vec.y(), lat1 + vec.x()));

                    vec *= 10;
                    points.push_back(QPointF(lon0 - vec.y(), lat0 + vec.x()));

                    QPolygonF polygon(points);
                    painter->drawPolygon(polygon);
                }
            }
        }
    }
}
