#ifndef VALUE_PROVIDER_HPP
#define VALUE_PROVIDER_HPP

#include <QString>
#include <QDoubleSpinBox>
#include "my/my_random.hpp"
#include "my/my_distributions.hpp"

namespace Ui { class GeneratorDialog; }

class ValueProvider {
public:
    virtual ~ValueProvider() = default;
    virtual double valueAt(int index, LCG& rng) = 0;
    // Для вещественного аргумента (по умолчанию вызывает целочисленный)
    virtual double valueAt(double x, LCG& rng) {
        return valueAt(static_cast<int>(x), rng);
    }
    virtual QString description() const = 0;
    virtual void configureUI(Ui::GeneratorDialog* ui, bool visible) const = 0;
    virtual bool isIndexDependent() const { return false; }
};

// ----- Функции -----
class LinearProvider : public ValueProvider {
    mutable QDoubleSpinBox* kSpin;
    mutable QDoubleSpinBox* bSpin;
public:
    LinearProvider(Ui::GeneratorDialog* ui);
    double valueAt(int index, LCG&) override;
    double valueAt(double x, LCG&) override;
    QString description() const override { return "y = k*x + b"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
    bool isIndexDependent() const override { return true; }
};

class QuadraticProvider : public ValueProvider {
    mutable QDoubleSpinBox* aSpin, *bSpin, *cSpin;
public:
    QuadraticProvider(Ui::GeneratorDialog* ui);
    double valueAt(int index, LCG&) override;
    double valueAt(double x, LCG&) override;
    QString description() const override { return "y = a*x² + b*x + c"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
    bool isIndexDependent() const override { return true; }
};

class SinusoidalProvider : public ValueProvider {
    mutable QDoubleSpinBox* ampSpin, *freqSpin, *phaseSpin;
public:
    SinusoidalProvider(Ui::GeneratorDialog* ui);
    double valueAt(int index, LCG&) override;
    double valueAt(double x, LCG&) override;
    QString description() const override { return "y = A*sin(freq*x + phase)"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
    bool isIndexDependent() const override { return true; }
};

class NoiseProvider : public ValueProvider {
    mutable QDoubleSpinBox* stdSpin;
public:
    NoiseProvider(Ui::GeneratorDialog* ui);
    double valueAt(int, LCG& rng) override;
    QString description() const override { return "Нормальный шум (0, σ)"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
};

// ----- Распределения -----
class UniformProvider : public ValueProvider {
    mutable QDoubleSpinBox* lowSpin, *highSpin;
public:
    UniformProvider(Ui::GeneratorDialog* ui);
    double valueAt(int, LCG& rng) override;
    QString description() const override { return "Равномерное [a, b]"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
};

class NormalProvider : public ValueProvider {
    mutable QDoubleSpinBox* muSpin, *sigmaSpin;
public:
    NormalProvider(Ui::GeneratorDialog* ui);
    double valueAt(int, LCG& rng) override;
    QString description() const override { return "Нормальное (μ, σ)"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
};

class ExponentialProvider : public ValueProvider {
    mutable QDoubleSpinBox* lambdaSpin;
public:
    ExponentialProvider(Ui::GeneratorDialog* ui);
    double valueAt(int, LCG& rng) override;
    QString description() const override { return "Экспоненциальное (λ)"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
};

class PoissonProvider : public ValueProvider {
    mutable QDoubleSpinBox* lambdaSpin;
public:
    PoissonProvider(Ui::GeneratorDialog* ui);
    double valueAt(int, LCG& rng) override;
    QString description() const override { return "Пуассона (λ)"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
};

class CauchyProvider : public ValueProvider {
    mutable QDoubleSpinBox* x0Spin, *gammaSpin;
public:
    CauchyProvider(Ui::GeneratorDialog* ui);
    double valueAt(int, LCG& rng) override;
    QString description() const override { return "Коши (x₀, γ)"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
};

class BernoulliProvider : public ValueProvider {
    mutable QDoubleSpinBox* pSpin;
public:
    BernoulliProvider(Ui::GeneratorDialog* ui);
    double valueAt(int, LCG& rng) override;
    QString description() const override { return "Бернулли (p) → 0 или 1"; }
    void configureUI(Ui::GeneratorDialog* ui, bool visible) const override;
};

#endif