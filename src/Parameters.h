#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "PriorityGroup.h"
#include <boost/shared_ptr.hpp>
#include <QtGui>

class Parameters : public QObject
{
    Q_OBJECT

    public:

        Parameters();

        double getR0();
        double getBetaScale();
        double getTau();
        double getKappa();
        double getChi();
        double getGamma();
        double getNu();
        double getAntiviralEffectiveness();
        double getAntiviralAdherence();
        double getAntiviralCapacity();
        double getVaccineEffectiveness();
        int getVaccineLatencyPeriod();
        double getVaccineAdherence();
        double getVaccineCapacity();

        // for parameters not exposed through ParametersWidget
        std::vector<boost::shared_ptr<PriorityGroup> > getPriorityGroups();

    signals:

        // for parameters not exposed through ParametersWidget
        void priorityGroupAdded(boost::shared_ptr<PriorityGroup> priorityGroup);

    public slots:

        void setR0(double value);
        void setBetaScale(double value);
        void setTau(double value);
        void setKappa(double value);
        void setChi(double value);
        void setGamma(double value);
        void setNu(double value);
        void setAntiviralEffectiveness(double value);
        void setAntiviralAdherence(double value);
        void setAntiviralCapacity(double value);
        void setVaccineEffectiveness(double value);
        void setVaccineLatencyPeriod(int value);
        void setVaccineAdherence(double value);
        void setVaccineCapacity(double value);

        // for parameters not exposed through ParametersWidget
        void addPriorityGroup(boost::shared_ptr<PriorityGroup> priorityGroup);

    private:

        // basic reproduction number
        double R0_;

        // scaling factor for beta (transmission rate given contact). beta = R0 / betaScale
        double betaScale_;

        // exposed -> asymptomatic transition rate
        double tau_;

        // asymptomatic -> treatable transition rate
        double kappa_;

        // time spent before progressing from treatable to infectious
        double chi_;

        // asymptomatic, treatable, or infectious -> recovered transition rate
        double gamma_;

        // asymptomatic, treatable, or infectious -> deceased transition rate
        // todo: this should be age stratified
        double nu_;

        // antiviral effectiveness
        double antiviralEffectiveness_;

        // antiviral adherence
        double antiviralAdherence_;

        // antiviral capacity (possible distributions per day as a fraction of total population)
        double antiviralCapacity_;

        // vaccine effectiveness
        double vaccineEffectiveness_;

        // vaccine latency period
        int vaccineLatencyPeriod_;

        // vaccine adherence
        double vaccineAdherence_;

        // vaccine capacity (possible distributions per day as a fraction of total population)
        double vaccineCapacity_;

        //////////////////////////////////////////////////////////////////
        // the parameters below are not exposed through ParametersWidget!
        //////////////////////////////////////////////////////////////////

        // priority groups
        std::vector<boost::shared_ptr<PriorityGroup> > priorityGroups_;
};

// global parameters object
extern Parameters g_parameters;

#endif
