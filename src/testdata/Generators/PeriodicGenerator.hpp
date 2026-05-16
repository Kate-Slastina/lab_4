#pragma once
#include <cmath>
#include <functional>

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
    std::function<double(double)> waveformFunc_;

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
        : waveform_(wf), amplitude_(amplitude), frequency_(frequency), phase_(phase), offset_(offset), index_(0)
    {
        switch (wf) {
            case Waveform::Sine:     waveformFunc_ = sineFunc; break;
            case Waveform::Square:   waveformFunc_ = squareFunc; break;
            case Waveform::Sawtooth: waveformFunc_ = sawtoothFunc; break;
            case Waveform::Triangle: waveformFunc_ = triangleFunc; break;
        }
    }

    T next() {
        double t = static_cast<double>(index_);
        double argument = 2.0 * M_PI * frequency_ * t + phase_;
        double value = amplitude_ * waveformFunc_(argument) + offset_;
        ++index_;
        return static_cast<T>(value);
    }

    T operator()() { return next(); }
    void reset() { index_ = 0; }
    size_t index() const { return index_; }
};