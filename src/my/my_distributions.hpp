#ifndef MY_DISTRIBUTIONS_HPP
#define MY_DISTRIBUTIONS_HPP

#include "my_random.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class UniformDist {
    double low, high;
public:
    UniformDist(double a, double b) : low(a), high(b) {}
    double operator()(LCG& rng) {
        return low + rng.nextDouble() * (high - low);
    }
};

class NormalDist {
    double mean, stddev;
    mutable double spare;
    mutable bool hasSpare;
public:
    NormalDist(double m, double s) : mean(m), stddev(s), spare(0), hasSpare(false) {}
    double operator()(LCG& rng) {
        if (hasSpare) {
            hasSpare = false;
            return mean + stddev * spare;
        }
        double u, v, s;
        do {
            u = rng.nextDouble() * 2.0 - 1.0;
            v = rng.nextDouble() * 2.0 - 1.0;
            s = u * u + v * v;
        } while (s >= 1.0 || s == 0.0);
        double mult = std::sqrt(-2.0 * std::log(s) / s);
        spare = v * mult;
        hasSpare = true;
        return mean + stddev * (u * mult);
    }
};

class ExponentialDist {
    double lambda;
public:
    ExponentialDist(double lam) : lambda(lam) {}
    double operator()(LCG& rng) {
        double u = rng.nextDouble();
        return -std::log(1.0 - u) / lambda;
    }
};

class PoissonDist {
    double lambda;
public:
    PoissonDist(double lam) : lambda(lam) {}
    int operator()(LCG& rng) {
        double L = std::exp(-lambda);
        double p = 1.0;
        int k = 0;
        do {
            k++;
            p *= rng.nextDouble();
        } while (p > L);
        return k - 1;
    }
};

class CauchyDist {
    double x0, gamma;
public:
    CauchyDist(double x0_, double gamma_) : x0(x0_), gamma(gamma_) {}
    double operator()(LCG& rng) {
        double u = rng.nextDouble();
        return x0 + gamma * std::tan(M_PI * (u - 0.5));
    }
};

class BernoulliDist {
    double p;
public:
    BernoulliDist(double prob) : p(prob) {}
    int operator()(LCG& rng) {
        return rng.nextDouble() < p ? 1 : 0;
    }
};

#endif