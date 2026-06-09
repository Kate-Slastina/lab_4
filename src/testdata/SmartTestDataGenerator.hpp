#pragma once
#include "../lazy/LazySequence.hpp"
#include "Distribution.hpp"
#include "SequenceProperty.hpp"
#include "../my/my_function.hpp"

template<typename T>
class SmartTestDataGenerator {
public:
    static LazySequence<T> Generate(
        size_t length,
        DistributionType distributionType,
        const   Array<double>& distParams,
        const   Array<SequenceProperty>& properties,
        double outlierProbability = 0.0,
        double outlierFactor = 10.0,
          Function<T(double)> converter = nullptr
    );
};

// Реализация в .cpp