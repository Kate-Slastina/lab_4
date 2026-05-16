#pragma once
#include <functional>
#include <vector>
#include <cmath>

enum class PropertyType {
    None,
    LinearTrend,
    QuadraticTrend,
    Sinusoidal,
    Noise,
    Outliers,
    CyclicShift
};

struct SequenceProperty {
    PropertyType type;
    std::vector<double> parameters;

    SequenceProperty(PropertyType t, const std::vector<double>& params = {})
        : type(t), parameters(params) {}

    double Apply(size_t index, double originalValue) const {
        double result = originalValue;
        switch (type) {
            case PropertyType::LinearTrend:
                if (parameters.size() >= 1)
                    result += parameters[0] * index;
                break;
            case PropertyType::QuadraticTrend:
                if (parameters.size() >= 1)
                    result += parameters[0] * index * index;
                break;
            case PropertyType::Sinusoidal:
                if (parameters.size() >= 3) {
                    double A = parameters[0];
                    double freq = parameters[1];
                    double phase = parameters[2];
                    result += A * std::sin(2.0 * M_PI * freq * index + phase);
                }
                break;
            default: break;
        }
        return result;
    }
};