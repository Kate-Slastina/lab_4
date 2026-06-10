#include "generatordialog.h"
#include "value_provider.hpp"
#include "ui_generatordialog.h"
#include <QDebug>
#include <QMessageBox>
#include <cmath>
#include <ctime>

static double uniformDensity(double x, double a, double b) {
    return (x >= a && x <= b) ? 1.0 / (b - a) : 0.0;
}
static double normalDensity(double x, double mu, double sigma) {
    double z = (x - mu) / sigma;
    return std::exp(-0.5 * z * z) / (sigma * std::sqrt(2.0 * M_PI));
}
static double exponentialDensity(double x, double lambda) {
    return (x >= 0) ? lambda * std::exp(-lambda * x) : 0.0;
}
static double poissonProbability(int k, double lambda) {
    if (k < 0) return 0.0;
    return std::pow(lambda, k) * std::exp(-lambda) / std::tgamma(k + 1);
}
static double cauchyDensity(double x, double x0, double gamma) {
    double t = (x - x0) / gamma;
    return 1.0 / (M_PI * gamma * (1.0 + t * t));
}
static double bernoulliProbability(int k, double p) {
    if (k == 0) return 1.0 - p;
    if (k == 1) return p;
    return 0.0;
}

GeneratorDialog::GeneratorDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::GeneratorDialog), chartView(nullptr), rng(42) {
    ui->setupUi(this);

    chartView = new QChartView(this);
    chartView->setMinimumHeight(200);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setContentsMargins(10, 10, 10, 10);
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->verticalLayoutWidget->layout());
    if (mainLayout) mainLayout->addWidget(chartView);

    ui->comboBox->addItem("Линейная");
    ui->comboBox->addItem("Квадратичная");
    ui->comboBox->addItem("Синусоидальная");
    ui->comboBox->addItem("Шум (нормальный)");
    ui->comboBox->addItem("Равномерное (Uniform)");
    ui->comboBox->addItem("Нормальное (Normal)");
    ui->comboBox->addItem("Экспоненциальное (Exponential)");
    ui->comboBox->addItem("Пуассона (Poisson)");
    ui->comboBox->addItem("Коши (Cauchy)");
    ui->comboBox->addItem("Бернулли (Bernoulli)");

    ui->lineEdit_1->setReadOnly(true);
    ui->doubleSpinBoxStep->setRange(0.01, 1.0);
    ui->doubleSpinBoxStep->setValue(1.0);
    ui->doubleSpinBoxStep->setSingleStep(0.01);

    initProviders();
    currentProvider = 0;
    updateProviderUI();

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx){ currentProvider = idx; updateProviderUI(); updatePreview(); });
    connect(ui->doubleSpinBoxStep, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->buttonCancel, &QPushButton::clicked, this, &GeneratorDialog::reject);
    connect(ui->buttonGenerate, &QPushButton::clicked, this, &GeneratorDialog::accept);

    connect(ui->doubleSpinBoxLinearK, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxLinearB, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxQuadA, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxQuadB, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxQuadC, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxSinAmp, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxSinFreq, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxSinPhase, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxNoiseStd, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxUniformLow, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxUniformHigh, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxNormalMu, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxNormalSigma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxExponentialLambda, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxPoissonLambda, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxCauchyX0, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxCauchyGamma, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);
    connect(ui->doubleSpinBoxBernoulliP, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &GeneratorDialog::updatePreview);

    updatePreview();
}

GeneratorDialog::~GeneratorDialog() { delete ui; }

void GeneratorDialog::initProviders() {
    providers.push_back(SharedPtr<ValueProvider>(new LinearProvider(ui)));
    providers.push_back(SharedPtr<ValueProvider>(new QuadraticProvider(ui)));
    providers.push_back(SharedPtr<ValueProvider>(new SinusoidalProvider(ui)));
    providers.push_back(SharedPtr<ValueProvider>(new NoiseProvider(ui)));
    providers.push_back(SharedPtr<ValueProvider>(new UniformProvider(ui)));
    providers.push_back(SharedPtr<ValueProvider>(new NormalProvider(ui)));
    providers.push_back(SharedPtr<ValueProvider>(new ExponentialProvider(ui)));
    providers.push_back(SharedPtr<ValueProvider>(new PoissonProvider(ui)));
    providers.push_back(SharedPtr<ValueProvider>(new CauchyProvider(ui)));
    providers.push_back(SharedPtr<ValueProvider>(new BernoulliProvider(ui)));
}

void GeneratorDialog::updateProviderUI() {
    for (size_t i = 0; i < providers.size(); ++i)
        providers[i]->configureUI(ui, false);
    if (currentProvider >= 0 && currentProvider < (int)providers.size())
        providers[currentProvider]->configureUI(ui, true);
    ui->lineEdit_1->setText(providers[currentProvider]->description());
}

void GeneratorDialog::generatePreview(int maxPoints) {
    if (!chartView) return;
    int points = maxPoints;
    QLineSeries *series = new QLineSeries();

    int type = currentProvider;
    if (type >= 4) {
        switch (type) {
        case 4: {
            double a = ui->doubleSpinBoxUniformLow->value();
            double b = ui->doubleSpinBoxUniformHigh->value();
            if (a >= b) std::swap(a, b);
            double xMin = a - 0.1 * (b - a);
            double xMax = b + 0.1 * (b - a);
            double step = (xMax - xMin) / points;
            for (int i = 0; i <= points; ++i) {
                double x = xMin + i * step;
                double y = uniformDensity(x, a, b);
                series->append(x, y);
            }
            break;
        }
        case 5: {
            double mu = ui->doubleSpinBoxNormalMu->value();
            double sigma = ui->doubleSpinBoxNormalSigma->value();
            if (sigma <= 0) sigma = 1.0;
            double xMin = mu - 5.0 * sigma;
            double xMax = mu + 5.0 * sigma;
            double step = (xMax - xMin) / points;
            for (int i = 0; i <= points; ++i) {
                double x = xMin + i * step;
                double y = normalDensity(x, mu, sigma);
                series->append(x, y);
            }
            break;
        }
        case 6: {
            double lambda = ui->doubleSpinBoxExponentialLambda->value();
            if (lambda <= 0) lambda = 1.0;
            double xMax = 5.0 / lambda;
            double step = xMax / points;
            for (int i = 0; i <= points; ++i) {
                double x = i * step;
                double y = exponentialDensity(x, lambda);
                series->append(x, y);
            }
            break;
        }
        case 7: {
            double lambda = ui->doubleSpinBoxPoissonLambda->value();
            if (lambda <= 0) lambda = 1.0;
            int maxK = static_cast<int>(lambda * 5 + 10);
            for (int k = 0; k <= maxK; ++k) {
                double y = poissonProbability(k, lambda);
                series->append(k, y);
            }
            break;
        }
        case 8: {
            double x0 = ui->doubleSpinBoxCauchyX0->value();
            double gamma = ui->doubleSpinBoxCauchyGamma->value();
            if (gamma <= 0) gamma = 1.0;
            double xMin = x0 - 10.0 * gamma;
            double xMax = x0 + 10.0 * gamma;
            double step = (xMax - xMin) / points;
            for (int i = 0; i <= points; ++i) {
                double x = xMin + i * step;
                double y = cauchyDensity(x, x0, gamma);
                series->append(x, y);
            }
            break;
        }
        case 9: {
            double p = ui->doubleSpinBoxBernoulliP->value();
            if (p < 0) p = 0;
            if (p > 1) p = 1;
            series->append(0, 1.0 - p);
            series->append(1, p);
            break;
        }
        }
    } else {
        ValueProvider& prov = *providers[type];
        double step = ui->doubleSpinBoxStep->value();
        for (int i = 0; i < points; ++i) {
            double x = i * step;
            double val = prov.valueAt(x, rng);
            series->append(x, val);
        }
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    QList<QAbstractAxis*> axesX = chart->axes(Qt::Horizontal);
    QList<QAbstractAxis*> axesY = chart->axes(Qt::Vertical);
    if (!axesX.isEmpty()) axesX.first()->setTitleText("x");
    if (!axesY.isEmpty()) axesY.first()->setTitleText("f(x)");
    if (type >= 4 && type != 7 && type != 9) {
        chart->setTitle("Плотность распределения");
    } else if (type == 7) {
        chart->setTitle("Функция вероятности Пуассона");
    } else if (type == 9) {
        chart->setTitle("Функция вероятности Бернулли");
    } else {
        double step = ui->doubleSpinBoxStep->value();
        chart->setTitle(QString("Предпросмотр (%1 точек, шаг=%2)").arg(points).arg(step));
    }
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
}

void GeneratorDialog::updatePreview() {
    generatePreview(1000);
}

// Исправленный getGeneratedSequence с нестатическими RNG
SharedPtr<LazySequence<double>> GeneratorDialog::getGeneratedSequence() const {
    double step = ui->doubleSpinBoxStep->value();
    int type = currentProvider;

    if (type >= 0 && type <= 3) {
        double p1 = 0.0, p2 = 0.0, p3 = 0.0;
        switch (type) {
        case 0: p1 = ui->doubleSpinBoxLinearK->value(); p2 = ui->doubleSpinBoxLinearB->value(); break;
        case 1: p1 = ui->doubleSpinBoxQuadA->value(); p2 = ui->doubleSpinBoxQuadB->value(); p3 = ui->doubleSpinBoxQuadC->value(); break;
        case 2: p1 = ui->doubleSpinBoxSinAmp->value(); p2 = ui->doubleSpinBoxSinFreq->value(); p3 = ui->doubleSpinBoxSinPhase->value(); break;
        case 3: p1 = ui->doubleSpinBoxNoiseStd->value(); break;
        }
        struct FunctionGen {
            int funcType;
            double a, b, c;
            double step;
            mutable LCG rng;
            FunctionGen(int type, double p1, double p2, double p3, double st, unsigned seed = static_cast<unsigned>(std::time(nullptr)))
                : funcType(type), a(p1), b(p2), c(p3), step(st), rng(seed) {}
            double operator()(size_t idx) const {
                double x = idx * step;
                switch (funcType) {
                case 0: return a * x + b;
                case 1: return a * x * x + b * x + c;
                case 2: return a * std::sin(b * x + c);
                case 3: {
                    NormalDist noise(0.0, a);
                    return noise(rng);
                }
                default: return 0.0;
                }
            }
        };
        FunctionGen gen(type, p1, p2, p3, step);
        return SharedPtr<LazySequence<double>>(new LazySequence<double>(Function<double(size_t)>(gen)));
    } else {
        struct DistributionGen {
            int distType;
            double param1, param2;
            mutable LCG rng;
            DistributionGen(int type, double p1, double p2, unsigned seed = static_cast<unsigned>(std::time(nullptr)))
                : distType(type), param1(p1), param2(p2), rng(seed) {}
            double operator()(size_t) const {
                switch (distType) {
                case 4: {
                    double low = param1, high = param2;
                    if (low >= high) std::swap(low, high);
                    UniformDist dist(low, high);
                    return dist(rng);
                }
                case 5: {
                    double mu = param1, sigma = param2;
                    if (sigma <= 0) sigma = 1.0;
                    NormalDist dist(mu, sigma);
                    return dist(rng);
                }
                case 6: {
                    double lambda = param1;
                    if (lambda <= 0) lambda = 1.0;
                    ExponentialDist dist(lambda);
                    return dist(rng);
                }
                case 7: {
                    double lambda = param1;
                    if (lambda <= 0) lambda = 1.0;
                    PoissonDist dist(lambda);
                    return static_cast<double>(dist(rng));
                }
                case 8: {
                    double x0 = param1, gamma = param2;
                    if (gamma <= 0) gamma = 1.0;
                    CauchyDist dist(x0, gamma);
                    return dist(rng);
                }
                case 9: {
                    double p = param1;
                    if (p < 0) p = 0;
                    if (p > 1) p = 1;
                    BernoulliDist dist(p);
                    return static_cast<double>(dist(rng));
                }
                default: return 0.0;
                }
            }
        };
        double p1 = 0.0, p2 = 1.0;
        switch (type) {
        case 4: p1 = ui->doubleSpinBoxUniformLow->value(); p2 = ui->doubleSpinBoxUniformHigh->value(); break;
        case 5: p1 = ui->doubleSpinBoxNormalMu->value(); p2 = ui->doubleSpinBoxNormalSigma->value(); break;
        case 6: p1 = ui->doubleSpinBoxExponentialLambda->value(); break;
        case 7: p1 = ui->doubleSpinBoxPoissonLambda->value(); break;
        case 8: p1 = ui->doubleSpinBoxCauchyX0->value(); p2 = ui->doubleSpinBoxCauchyGamma->value(); break;
        case 9: p1 = ui->doubleSpinBoxBernoulliP->value(); break;
        default: break;
        }
        DistributionGen gen(type, p1, p2);
        return SharedPtr<LazySequence<double>>(new LazySequence<double>(Function<double(size_t)>(gen)));
    }
}