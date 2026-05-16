#pragma once
#include <memory>
#include <functional>
#include <random>
#include "lazy/LazySequence.hpp"
#include "Distribution.hpp"
#include "SequenceProperty.hpp"

template<typename T>
class SmartTestDataGenerator {
public:
    static LazySequence<T> Generate(
        size_t length,
        DistributionType distributionType,
        const std::vector<double>& distParams,
        const std::vector<SequenceProperty>& properties,
        double outlierProbability = 0.0,
        double outlierFactor = 10.0,
        std::function<T(double)> converter = [](double d) { return static_cast<T>(d); }
    );

    static LazySequence<T> GenerateRandomized(size_t length, unsigned seed = std::random_device{}());

private:
    static double GenerateBaseValue(std::mt19937& rng, Distribution& dist);
    static double ApplyProperties(size_t index, double baseValue, const std::vector<SequenceProperty>& props,
                                  std::mt19937& rng, double outlierProb, double outlierFactor);
};