#ifndef PARAMETERS_WIDGET_H
#define PARAMETERS_WIDGET_H

#define WIDGET_MAX_VALUE 999999999.
#define WIDGET_NUM_DECIMALS 6

#include <QtGui>

class ParametersWidget : public QScrollArea
{
    Q_OBJECT

    public:

        ParametersWidget();

    public slots:

        // read values from Parameters:: class and set widget values
        void readValues();

        void setR0(double value);
        void setBetaScale(double value);
        void setTau(double value);
        void setKappa(double value);
        void setChi(double value);
        void setGamma(double value);
        void setNu(double value);
        void setAntiviralEffectiveness(double value);

    private:

        QDoubleSpinBox R0_spinBox_;
        QDoubleSpinBox betaScale_spinBox_;
        QDoubleSpinBox tau_spinBox_;
        QDoubleSpinBox kappa_spinBox_;
        QDoubleSpinBox chi_spinBox_;
        QDoubleSpinBox gamma_spinBox_;
        QDoubleSpinBox nu_spinBox_;
        QDoubleSpinBox antiviralEffectiveness_spinBox_;
};

#endif
