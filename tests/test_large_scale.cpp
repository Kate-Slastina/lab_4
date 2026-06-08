#include <gtest/gtest.h>
#include <fstream>
#include <random>
#include "lazy/LazySequence.hpp"
#include "stream/FileStream.hpp"
#include "stream/ReadOnlyStream.hpp"

// Генерация миллиона элементов
TEST(LargeScale, MillionElementsLazySequence) {
    // генерируем последовательность длиной 1e6 через итератор
    std::function<double()> gen = []() {
        static std::mt19937 rng(42);
        static std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng);
    };
    const size_t N = 1'000'000;
    LazySequence<double> seq(gen, N);
    EXPECT_EQ(seq.GetLength(), N);
    // читаем последний элемент, чтобы материализовать всё
    double last = seq.Get(N - 1);
    EXPECT_GE(last, 0.0);
    EXPECT_LE(last, 1.0);
    // проверка, что кэш не превышает разумный предел (у нас он будет N)
    // Это нормально для одного теста, память ~ 8 MB для double.
}

// Создание временного большого файла (200 МБ) и чтение через FileStream
TEST(LargeScale, LargeFileStream) {
    const std::string filename = "temp_large_test.txt";
    const size_t lines = 10'000'000;  // 10 млн строк -> примерно 200-300 MB
    {
        std::ofstream out(filename);
        if (!out) FAIL() << "Cannot create temp file";
        for (size_t i = 0; i < lines; ++i) {
            out << i << "\n";
        }
    }
    FileStream<int> fs(filename, Deserializer<int>(), true);
    fs.Open();
    size_t count = 0;
    while (!fs.IsEndOfStream()) {
        int val = fs.Read();
        EXPECT_EQ(val, count);
        ++count;
        if (count % 1000000 == 0) {
            std::cout << "Read " << count/1000000 << "M lines\n";
        }
    }
    EXPECT_EQ(count, lines);
    fs.Close();
    std::remove(filename.c_str());
}

// Тест на производительность Concat для больших последовательностей
TEST(LargeScale, ConcatLarge) {
    const size_t N = 500'000;
    std::function<int()> gen = []() { return 1; };
    LazySequence<int> seq1(gen, N);
    LazySequence<int> seq2(gen, N);
    auto* concat = seq1.Concat(&seq2);
    EXPECT_EQ(concat->GetLength(), 2*N);
    int sum = 0;
    for (size_t i = 0; i < 2*N; ++i) {
        sum += concat->Get(i);
    }
    EXPECT_EQ(sum, 2*N);
    delete concat;
}