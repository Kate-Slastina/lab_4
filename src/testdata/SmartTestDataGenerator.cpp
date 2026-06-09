#include "SmartTestDataGenerator.hpp"

template<typename T>
LazySequence<T> SmartTestDataGenerator<T>::Generate(
    size_t length,
    DistributionType distributionType,
    const   Array<double>& distParams,
    const   Array<SequenceProperty>& properties,
    double outlierProbability,
    double outlierFactor,
      Function<T(double)> converter)
{
    Distribution* dist = CreateDistribution(distributionType,
                                            distParams.size() > 0 ? distParams[0] : 0.0,
                                            distParams.size() > 1 ? distParams[1] : 1.0);
      LCG rng(42); // можно случайный seed, но для детерминизма оставим 42

    auto generatorFunc = [dist, properties, outlierProbability, outlierFactor, converter, rng]() mutable -> T {
        static size_t index = 0;
        double base = dist->Generate(rng);
        double modified = base;
        for (size_t i = 0; i < properties.size(); ++i) {
            if (properties[i].type != PropertyType::Outliers)
                modified = properties[i].Apply(index, modified, rng);
        }
        // выбросы
        if (outlierProbability > 0.0) {
              BernoulliDist outlierDist(outlierProbability);
            if (outlierDist(rng)) modified *= outlierFactor;
        }
        ++index;
        if (converter) return converter(modified);
        else return static_cast<T>(modified);
    };

    if (length > 0) {
        return LazySequence<T>(generatorFunc, length);
    } else {
        return LazySequence<T>(generatorFunc);
    }
}

template class SmartTestDataGenerator<int>;
template class SmartTestDataGenerator<double>;
template class SmartTestDataGenerator<float>;