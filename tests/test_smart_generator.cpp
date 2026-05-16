#include <gtest/gtest.h>
#include "testdata/SmartTestDataGenerator.hpp"
#include "testdata/Distribution.hpp"
#include "testdata/SequenceProperty.hpp"

// Тест 1: Генерация конечной последовательности с равномерным распределением
TEST(SmartGeneratorTest, GenerateUniformFinite) {
    std::vector<SequenceProperty> props;
    auto seq = SmartTestDataGenerator<double>::Generate(
        1000,                     // длина
        DistributionType::Uniform,
        {0.0, 100.0},            // диапазон [0,100]
        props,
        0.0, 0.0                 // без выбросов
    );
    EXPECT_EQ(seq.GetLength(), 1000);
    // Проверяем, что все значения в диапазоне [0,100]
    for (size_t i = 0; i < 1000; ++i) {
        double val = seq.Get(i);
        EXPECT_GE(val, 0.0);
        EXPECT_LE(val, 100.0);
    }
}

// Тест 2: Бесконечная генерация (проверяем только первые 500 элементов)
TEST(SmartGeneratorTest, GenerateInfinite) {
    auto seq = SmartTestDataGenerator<double>::Generate(
        0,                        // бесконечная
        DistributionType::Normal,
        {0.0, 1.0},
        {},
        0.0, 0.0
    );
    EXPECT_FALSE(seq.IsFinite());
    for (size_t i = 0; i < 500; ++i) {
        EXPECT_NO_THROW(seq.Get(i));
    }
}

// Тест 3: Линейный тренд + шум
TEST(SmartGeneratorTest, LinearTrendWithNoise) {
    std::vector<SequenceProperty> props;
    props.emplace_back(PropertyType::LinearTrend, std::vector<double>{0.5}); // slope = 0.5
    props.emplace_back(PropertyType::Noise, std::vector<double>{0.2});       // шум sigma=0.2

    auto seq = SmartTestDataGenerator<double>::Generate(
        100,
        DistributionType::Uniform,
        {0.0, 10.0},
        props,
        0.0, 0.0
    );
    // Проверяем, что среднее значение растёт (грубая проверка тренда)
    double sumFirstHalf = 0.0, sumSecondHalf = 0.0;
    for (size_t i = 0; i < 50; ++i) sumFirstHalf += seq.Get(i);
    for (size_t i = 50; i < 100; ++i) sumSecondHalf += seq.Get(i);
    EXPECT_GT(sumSecondHalf / 50.0, sumFirstHalf / 50.0);
}

// Тест 4: Выбросы – проверка вероятности
TEST(SmartGeneratorTest, Outliers) {
    auto seq = SmartTestDataGenerator<double>::Generate(
        10000,
        DistributionType::Normal,
        {0.0, 1.0},
        {},
        0.05,     // 5% выбросов
        10.0
    );
    int outlierCount = 0;
    for (size_t i = 0; i < 10000; ++i) {
        double val = seq.Get(i);
        if (std::abs(val) > 5.0) // нормальное распределение редко даёт >5 сигм, выбросы >10*1 =10, но упростим
            outlierCount++;
    }
    // Ожидаем около 500 выбросов (5% от 10000), допустим разброс
    EXPECT_GT(outlierCount, 200);
    EXPECT_LT(outlierCount, 1500);
}

// Тест 5: Различные типы распределений (проверяем, что не падает)
TEST(SmartGeneratorTest, AllDistributionTypes) {
    std::vector<DistributionType> types = {
        DistributionType::Uniform,
        DistributionType::Normal,
        DistributionType::Exponential,
        DistributionType::Poisson,
        DistributionType::Cauchy,
        DistributionType::Bernoulli
    };
    for (auto type : types) {
        EXPECT_NO_THROW({
            auto seq = SmartTestDataGenerator<double>::Generate(50, type, {1.0, 2.0}, {}, 0.0, 0.0);
            seq.Get(49); // материализовать последний элемент
        });
    }
}

// Тест 6: Генерация с преобразователем типа (int)
TEST(SmartGeneratorTest, ConversionToInt) {
    auto conv = [](double d) { return static_cast<int>(std::round(d)); };
    auto seq = SmartTestDataGenerator<int>::Generate(
        100,
        DistributionType::Uniform,
        {0.0, 10.0},
        {},
        0.0, 0.0,
        conv
    );
    for (size_t i = 0; i < 100; ++i) {
        int val = seq.Get(i);
        EXPECT_GE(val, 0);
        EXPECT_LE(val, 10);
    }
}