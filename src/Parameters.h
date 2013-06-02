#ifndef PARAMETERS_H
#define PARAMETERS_H

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
        double getVaccineEffectiveness();
        double getVaccineAdherence();

    public slots:

        void setR0(double value);
        void setBetaScale(double value);
        void setTau(double value);
        void setKappa(double value);
        void setChi(double value);
        void setGamma(double value);
        void setNu(double value);
        void setAntiviralEffectiveness(double value);
        void setVaccineEffectiveness(double value);
        void setVaccineAdherence(double value);

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

        // vaccine effectiveness
        double vaccineEffectiveness_;

        // vaccine adherence
        double vaccineAdherence_;
};

// global parameters object
extern Parameters g_parameters;

#endif
