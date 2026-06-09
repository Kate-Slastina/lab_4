#pragma once
#include "../my/my_random.hpp"
#include "../my/my_distributions.hpp"
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
    virtual double Generate(LCG& rng) = 0;
    virtual ~Distribution() = default;
};

class UniformDistribution : public Distribution {
    double a, b;
public:
    UniformDistribution(double low = 0.0, double high = 1.0) : a(low), b(high) {}
    double Generate(LCG& rng) override {
        UniformDist dist(a, b);
        return dist(rng);
    }
};

class NormalDistribution : public Distribution {
    double mu, sigma;
public:
    NormalDistribution(double mean = 0.0, double stddev = 1.0) : mu(mean), sigma(stddev) {}
    double Generate(LCG& rng) override {
        NormalDist dist(mu, sigma);
        return dist(rng);
    }
};

class ExponentialDistribution : public Distribution {
    double lambda;
public:
    ExponentialDistribution(double rate = 1.0) : lambda(rate) {}
    double Generate(LCG& rng) override {
        ExponentialDist dist(lambda);
        return dist(rng);
    }
};

class PoissonDistribution : public Distribution {
    double lambda;
public:
    PoissonDistribution(double mean = 1.0) : lambda(mean) {}
    double Generate(LCG& rng) override {
        PoissonDist dist(lambda);
        return static_cast<double>(dist(rng));
    }
};

class CauchyDistribution : public Distribution {
    double x0, gamma;
public:
    CauchyDistribution(double location = 0.0, double scale = 1.0) : x0(location), gamma(scale) {}
    double Generate(LCG& rng) override {
        CauchyDist dist(x0, gamma);
        return dist(rng);
    }
};

class BernoulliDistribution : public Distribution {
    double p;
public:
    BernoulliDistribution(double prob = 0.5) : p(prob) {}
    double Generate(LCG& rng) override {
        BernoulliDist dist(p);
        return static_cast<double>(dist(rng));
    }
};

std::shared_ptr<Distribution> CreateDistribution(DistributionType type, double param1 = 0.0, double param2 = 1.0) {
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