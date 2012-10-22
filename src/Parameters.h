#ifndef PARAMETERS_H
#define PARAMETERS_H

class Parameters
{
    public:

        Parameters();

        void setR0(double value);
        double getR0();

        void setBetaScale(double value);
        double getBetaScale();

        void setTau(double value);
        double getTau();

        void setKappa(double value);
        double getKappa();

        void setChi(double value);
        double getChi();

        void setGamma(double value);
        double getGamma();

        void setNu(double value);
        double getNu();

    private:

        // basic reproduction number
        double R0_;

        // scaling factor for beta (transmission rate given contact). beta = R0 / betaScale
        double betaScale_;

        // exposed -> asymptomatic transition rate
        double tau_;

        // asymptomatic -> treatable infectious transition rate
        double kappa_;

        // time spent before progressing from treatable to untreatable infectious
        double chi_;

        // asymptomatic or infectious -> recovered transition rate
        double gamma_;

        // asymptomatic or infectious -> deceased transition rate
        // todo: this should be age stratified
        double nu_;
};

// global parameters object
extern Parameters g_parameters;

#endif
