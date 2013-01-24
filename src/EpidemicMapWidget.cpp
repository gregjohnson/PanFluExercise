#include "EpidemicMapWidget.h"
#include "EpidemicDataSet.h"
#include "MapShape.h"

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
            float infectiousFraction = dataSet_->getValue("infectious", time_, iter->first) / dataSet_->getPopulation(iter->first);

            // map to color
            float r, g, b;
            countiesColorMap_.getColor3(infectiousFraction / 0.01, r, g, b);

            iter->second->setColor(r, g, b);
        }

        // force redraw
        update();
    }
}

void EpidemicMapWidget::render()
{
    renderCountyShapes();
    renderCountyTravel();
}

void EpidemicMapWidget::renderCountyTravel()
{
    // parameters
    float infectiousTravelerThreshhold = 1.;
    float infectiousTravelerAlphaScale = 100.;

    if(dataSet_ == NULL)
    {
        return;
    }

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(3.);

    glBegin(GL_LINES);

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

                if(infectiousTravelers > infectiousTravelerThreshhold)
                {
                    glColor4f(1.,0.,0., infectiousTravelers / infectiousTravelerAlphaScale);

                    glVertex2f(lon0, lat0);
                    glVertex2f(lon1, lat1);
                }
            }
        }
    }

    glEnd();

    glPopAttrib();
}
