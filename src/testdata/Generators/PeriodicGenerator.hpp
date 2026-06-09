#pragma once
#include <cmath>

enum class Waveform {
    Sine,
    Square,
    Sawtooth,
    Triangle
};

template<typename T = double>
class PeriodicGenerator {
private:
    Waveform waveform_;
    double amplitude_;
    double frequency_;
    double phase_;
    double offset_;
    size_t index_;

    static double sineFunc(double x) { return std::sin(x); }
    static double squareFunc(double x) { return std::sin(x) >= 0 ? 1.0 : -1.0; }
    static double sawtoothFunc(double x) {
        double frac = x - std::floor(x);
        return 2.0 * frac - 1.0;
    }
    static double triangleFunc(double x) {
        double frac = x - std::floor(x);
        return 4.0 * std::abs(frac - 0.5) - 1.0;
    }

public:
    PeriodicGenerator(Waveform wf, double amplitude, double frequency, double phase = 0.0, double offset = 0.0)
        : waveform_(wf), amplitude_(amplitude), frequency_(frequency), phase_(phase), offset_(offset), index_(0) {}

    T next() {
        double t = static_cast<double>(index_);
        double argument = 2.0 * M_PI * frequency_ * t + phase_;
        double value;
        switch (waveform_) {
            case Waveform::Sine:     value = sineFunc(argument); break;
            case Waveform::Square:   value = squareFunc(argument); break;
            case Waveform::Sawtooth: value = sawtoothFunc(argument); break;
            case Waveform::Triangle: value = triangleFunc(argument); break;
            default: value = 0.0;
        }
        value = amplitude_ * value + offset_;
        ++index_;
        return static_cast<T>(value);
    }

    void reset() { index_ = 0; }
    T operator()() { return next(); }
    size_t index() const { return index_; }
};