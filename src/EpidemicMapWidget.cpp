#include "EpidemicMapWidget.h"
#include "EpidemicDataSet.h"
#include "MapShape.h"

EpidemicMapWidget::EpidemicMapWidget()
{
    setTitle("Infectious Population by County");
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
            float infectiousFraction = dataSet_->getValue("infectious", time_, iter->first) / dataSet_->getPopulation(iter->first);

            // map to color
            float r, g, b;
            countiesColorMap_.getColor3(infectiousFraction * 100, r, g, b);
			
			printf("rgb = %f %f %f\n", r,g,b);

            iter->second->setColor(r, g, b);
        }

        // force redraw
        update();
		
        exportSVGToDisplayCluster();
    }
}

void EpidemicMapWidget::render(QPainter* painter)
{
    renderCountyShapes(painter);
    renderCountyTravel(painter);
}

void EpidemicMapWidget::renderCountyTravel(QPainter* painter)
{

	//AARONBAD -- need to convert this to QPainter
    // parameters
    float infectiousTravelerThreshhold = 1.;
    float infectiousTravelerAlphaScale = 100.;

    if(dataSet_ == NULL)
    {
        return;
    }

    //glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);

    //glDisable(GL_DEPTH_TEST);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glLineWidth(3.);

    //glBegin(GL_LINES);

    painter->setPen(QPen(QBrush(QColor::fromRgbF(1, 0, 0, .1)), .07));

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
		   	     	//painter->drawLine(QPoint(lat0, lon0), QPoint(lat1, lon1));
					
                    //glColor4f(1.,0.,0., infectiousTravelers / infectiousTravelerAlphaScale);

                    //glVertex2f(lon0, lat0);
                    //glVertex2f(lon1, lat1);
					
					
				    QVector<QPointF> points;
			        points.push_back(QPointF(lon0, lat0));
			        points.push_back(QPointF(lon1, lat1));

				    QPolygonF polygon(points);
				    painter->drawPolygon(polygon);					
                }
            }
        }
    }

    glEnd();

    glPopAttrib();
}
