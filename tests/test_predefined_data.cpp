#include <gtest/gtest.h>
#include "lazy/LazySequence.hpp"
#include "testdata/SmartTestDataGenerator.hpp"
#include "testdata/SequenceProperty.hpp"

// Предопределённые коллекции
static const std::vector<double> predefinedLinear = {1.0, 2.0, 3.0, 4.0, 5.0};
static const std::vector<SequenceProperty> predefinedProps = {
    SequenceProperty(PropertyType::LinearTrend, {0.5}),
    SequenceProperty(PropertyType::Noise, {0.1})
};

TEST(PredefinedData, LinearArraySequence) {
    LazySequence<double> seq(predefinedLinear.data(), predefinedLinear.size());
    EXPECT_EQ(seq.GetLength(), 5);
    EXPECT_DOUBLE_EQ(seq.Get(0), 1.0);
    EXPECT_DOUBLE_EQ(seq.Get(4), 5.0);
}

TEST(PredefinedData, SmartGeneratorWithPredefinedProps) {
    auto seq = SmartTestDataGenerator<double>::Generate(
        100, DistributionType::Normal, {0.0, 1.0}, predefinedProps, 0.05, 10.0);
    EXPECT_TRUE(seq.IsFinite());
    EXPECT_EQ(seq.GetLength(), 100);
    // просто проверяем, что не брошено исключение
    for (size_t i = 0; i < 10; ++i) seq.Get(i);
}