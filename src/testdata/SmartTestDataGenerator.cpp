#include "SmartTestDataGenerator.hpp"
#include <algorithm>
#include "core/Exceptions.hpp"

template<typename T>
LazySequence<T> SmartTestDataGenerator<T>::Generate(
    size_t length,
    DistributionType distributionType,
    const std::vector<double>& distParams,
    const std::vector<SequenceProperty>& properties,
    double outlierProbability,
    double outlierFactor,
    std::function<T(double)> converter)
{
    auto dist = CreateDistribution(distributionType,
                                   distParams.size() > 0 ? distParams[0] : 0.0,
                                   distParams.size() > 1 ? distParams[1] : 1.0);

    std::random_device rd;
    std::mt19937 rng(rd());

    auto generatorFunc = [dist, properties, outlierProbability, outlierFactor, converter, rng]() mutable -> T {
        static size_t index = 0;
        double base = GenerateBaseValue(rng, *dist);
        double modified = ApplyProperties(index, base, properties, rng, outlierProbability, outlierFactor);
        ++index;
        return converter(modified);
    };

    if (length > 0) {
        return LazySequence<T>(generatorFunc, length);
    } else {
        return LazySequence<T>(generatorFunc);
    }
}

template<typename T>
double SmartTestDataGenerator<T>::GenerateBaseValue(std::mt19937& rng, Distribution& dist) {
    return dist.Generate(rng);
}

template<typename T>
double SmartTestDataGenerator<T>::ApplyProperties(size_t index, double baseValue,
                                                   const std::vector<SequenceProperty>& props,
                                                   std::mt19937& rng,
                                                   double outlierProb, double outlierFactor) {
    double result = baseValue;
    for (const auto& prop : props) {
        if (prop.type != PropertyType::Noise && prop.type != PropertyType::Outliers) {
            result = prop.Apply(index, result);
        }
    }
    for (const auto& prop : props) {
        if (prop.type == PropertyType::Noise && prop.parameters.size() >= 1) {
            double sigma = prop.parameters[0];
            std::normal_distribution<double> noiseDist(0.0, sigma);
            result += noiseDist(rng);
        }
    }
    if (outlierProb > 0.0) {
        std::bernoulli_distribution outlierDist(outlierProb);
        if (outlierDist(rng)) {
            result *= outlierFactor;
        }
    }
    return result;
}

template<typename T>
LazySequence<T> SmartTestDataGenerator<T>::GenerateRandomized(size_t length, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> typeDist(0, 4);
    DistributionType types[] = {DistributionType::Uniform, DistributionType::Normal,
                                DistributionType::Exponential, DistributionType::Poisson,
                                DistributionType::Cauchy};
    DistributionType chosenType = types[typeDist(rng) % 5];

    std::vector<double> distParams;
    switch (chosenType) {
        case DistributionType::Uniform: distParams = { -10.0, 10.0 }; break;
        case DistributionType::Normal:  distParams = { 0.0, 5.0 }; break;
        default: distParams = { 1.0 };
    }

    std::vector<SequenceProperty> props;
    std::uniform_real_distribution<double> trendProb(0.0, 1.0);
    if (trendProb(rng) < 0.5) {
        std::uniform_real_distribution<double> slope(-0.5, 0.5);
        props.emplace_back(PropertyType::LinearTrend, std::vector<double>{slope(rng)});
    }
    if (trendProb(rng) < 0.3) {
        std::uniform_real_distribution<double> noiseSigma(0.1, 2.0);
        props.emplace_back(PropertyType::Noise, std::vector<double>{noiseSigma(rng)});
    }

    double outlierProb = trendProb(rng) < 0.2 ? 0.05 : 0.0;
    return Generate(length, chosenType, distParams, props, outlierProb, 10.0);
}

template class SmartTestDataGenerator<int>;
template class SmartTestDataGenerator<double>;
template class SmartTestDataGenerator<float>;