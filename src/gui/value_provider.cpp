#include "value_provider.hpp"
#include "ui_generatordialog.h"
#include <cmath>

// ----- Linear -----
LinearProvider::LinearProvider(Ui::GeneratorDialog* ui)
    : kSpin(ui->doubleSpinBoxLinearK), bSpin(ui->doubleSpinBoxLinearB) {}
double LinearProvider::valueAt(int index, LCG&) {
    return kSpin->value() * index + bSpin->value();
}
double LinearProvider::valueAt(double x, LCG&) {
    return kSpin->value() * x + bSpin->value();
}
void LinearProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetLinearParams->setVisible(visible);
}

// ----- Quadratic -----
QuadraticProvider::QuadraticProvider(Ui::GeneratorDialog* ui)
    : aSpin(ui->doubleSpinBoxQuadA), bSpin(ui->doubleSpinBoxQuadB), cSpin(ui->doubleSpinBoxQuadC) {}
double QuadraticProvider::valueAt(int index, LCG&) {
    double a = aSpin->value(), b = bSpin->value(), c = cSpin->value();
    return a * index * index + b * index + c;
}
double QuadraticProvider::valueAt(double x, LCG&) {
    double a = aSpin->value(), b = bSpin->value(), c = cSpin->value();
    return a * x * x + b * x + c;
}
void QuadraticProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetQuadParams->setVisible(visible);
}

// ----- Sinusoidal -----
SinusoidalProvider::SinusoidalProvider(Ui::GeneratorDialog* ui)
    : ampSpin(ui->doubleSpinBoxSinAmp), freqSpin(ui->doubleSpinBoxSinFreq), phaseSpin(ui->doubleSpinBoxSinPhase) {}
double SinusoidalProvider::valueAt(int index, LCG&) {
    return ampSpin->value() * std::sin(freqSpin->value() * index + phaseSpin->value());
}
double SinusoidalProvider::valueAt(double x, LCG&) {
    return ampSpin->value() * std::sin(freqSpin->value() * x + phaseSpin->value());
}
void SinusoidalProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetSinParams->setVisible(visible);
}

// ----- Noise -----
NoiseProvider::NoiseProvider(Ui::GeneratorDialog* ui)
    : stdSpin(ui->doubleSpinBoxNoiseStd) {}
double NoiseProvider::valueAt(int, LCG& rng) {
    NormalDist noise(0.0, stdSpin->value());
    return noise(rng);
}
void NoiseProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetNoiseParams->setVisible(visible);
}

// ----- Uniform -----
UniformProvider::UniformProvider(Ui::GeneratorDialog* ui)
    : lowSpin(ui->doubleSpinBoxUniformLow), highSpin(ui->doubleSpinBoxUniformHigh) {}
double UniformProvider::valueAt(int, LCG& rng) {
    double low = lowSpin->value(), high = highSpin->value();
    if (low >= high) std::swap(low, high);
    UniformDist dist(low, high);
    return dist(rng);
}
void UniformProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetUniformParams->setVisible(visible);
}

// ----- Normal -----
NormalProvider::NormalProvider(Ui::GeneratorDialog* ui)
    : muSpin(ui->doubleSpinBoxNormalMu), sigmaSpin(ui->doubleSpinBoxNormalSigma) {}
double NormalProvider::valueAt(int, LCG& rng) {
    double mu = muSpin->value(), sigma = sigmaSpin->value();
    if (sigma <= 0) sigma = 1.0;
    NormalDist dist(mu, sigma);
    return dist(rng);
}
void NormalProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetNormalParams->setVisible(visible);
}

// ----- Exponential -----
ExponentialProvider::ExponentialProvider(Ui::GeneratorDialog* ui)
    : lambdaSpin(ui->doubleSpinBoxExponentialLambda) {}
double ExponentialProvider::valueAt(int, LCG& rng) {
    double lambda = lambdaSpin->value();
    if (lambda <= 0) lambda = 1.0;
    ExponentialDist dist(lambda);
    return dist(rng);
}
void ExponentialProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetExponentialParams->setVisible(visible);
}

// ----- Poisson -----
PoissonProvider::PoissonProvider(Ui::GeneratorDialog* ui)
    : lambdaSpin(ui->doubleSpinBoxPoissonLambda) {}
double PoissonProvider::valueAt(int, LCG& rng) {
    double lambda = lambdaSpin->value();
    if (lambda <= 0) lambda = 1.0;
    PoissonDist dist(lambda);
    return static_cast<double>(dist(rng));
}
void PoissonProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetPoissonParams->setVisible(visible);
}

// ----- Cauchy -----
CauchyProvider::CauchyProvider(Ui::GeneratorDialog* ui)
    : x0Spin(ui->doubleSpinBoxCauchyX0), gammaSpin(ui->doubleSpinBoxCauchyGamma) {}
double CauchyProvider::valueAt(int, LCG& rng) {
    double x0 = x0Spin->value(), gamma = gammaSpin->value();
    if (gamma <= 0) gamma = 1.0;
    CauchyDist dist(x0, gamma);
    return dist(rng);
}
void CauchyProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetCauchyParams->setVisible(visible);
}

// ----- Bernoulli -----
BernoulliProvider::BernoulliProvider(Ui::GeneratorDialog* ui)
    : pSpin(ui->doubleSpinBoxBernoulliP) {}
double BernoulliProvider::valueAt(int, LCG& rng) {
    double p = pSpin->value();
    if (p < 0) p = 0;
    if (p > 1) p = 1;
    BernoulliDist dist(p);
    return static_cast<double>(dist(rng));
}
void BernoulliProvider::configureUI(Ui::GeneratorDialog* ui, bool visible) const {
    ui->widgetBernoulliParams->setVisible(visible);
}