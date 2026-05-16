#pragma once
#include <random>
#include <memory>

enum class DistributionType {
    Uniform,
    Normal,
    Exponential,
    Poisson,
    Cauchy,
    Bernoulli
};

class Distribution {
public:
    virtual double Generate(std::mt19937& rng) = 0;
    virtual ~Distribution() = default;
};

class UniformDistribution : public Distribution {
    double a, b;
    std::uniform_real_distribution<double> dist;
public:
    UniformDistribution(double low = 0.0, double high = 1.0) : a(low), b(high), dist(low, high) {}
    double Generate(std::mt19937& rng) override { return dist(rng); }
};

class NormalDistribution : public Distribution {
    double mu, sigma;
    std::normal_distribution<double> dist;
public:
    NormalDistribution(double mean = 0.0, double stddev = 1.0) : mu(mean), sigma(stddev), dist(mean, stddev) {}
    double Generate(std::mt19937& rng) override { return dist(rng); }
};

class ExponentialDistribution : public Distribution {
    double lambda;
    std::exponential_distribution<double> dist;
public:
    ExponentialDistribution(double rate = 1.0) : lambda(rate), dist(rate) {}
    double Generate(std::mt19937& rng) override { return dist(rng); }
};

class PoissonDistribution : public Distribution {
    double lambda;
    std::poisson_distribution<int> dist;
public:
    PoissonDistribution(double mean = 1.0) : lambda(mean), dist(mean) {}
    double Generate(std::mt19937& rng) override { return static_cast<double>(dist(rng)); }
};

class CauchyDistribution : public Distribution {
    double x0, gamma;
    std::cauchy_distribution<double> dist;
public:
    CauchyDistribution(double location = 0.0, double scale = 1.0) : x0(location), gamma(scale), dist(location, scale) {}
    double Generate(std::mt19937& rng) override { return dist(rng); }
};

class BernoulliDistribution : public Distribution {
    double p;
    std::bernoulli_distribution dist;
public:
    BernoulliDistribution(double prob = 0.5) : p(prob), dist(prob) {}
    double Generate(std::mt19937& rng) override { return dist(rng) ? 1.0 : 0.0; }
};

inline std::shared_ptr<Distribution> CreateDistribution(DistributionType type, double param1 = 0.0, double param2 = 1.0) {
    switch (type) {
        case DistributionType::Uniform:    return std::make_shared<UniformDistribution>(param1, param2);
        case DistributionType::Normal:     return std::make_shared<NormalDistribution>(param1, param2);
        case DistributionType::Exponential:return std::make_shared<ExponentialDistribution>(param1);
        case DistributionType::Poisson:    return std::make_shared<PoissonDistribution>(param1);
        case DistributionType::Cauchy:     return std::make_shared<CauchyDistribution>(param1, param2);
        case DistributionType::Bernoulli:  return std::make_shared<BernoulliDistribution>(param1);
        default:                           return std::make_shared<UniformDistribution>();
    }
}