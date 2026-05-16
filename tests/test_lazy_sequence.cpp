#include <gtest/gtest.h>
#include "lazy/LazySequence.hpp"
#include "core/Sequence.hpp"
#include "core/Exceptions.hpp"

TEST(LazySequenceTest, EmptySequence) {
    LazySequence<int> seq;
    EXPECT_EQ(seq.GetLength(), 0);
    EXPECT_TRUE(seq.IsEmpty());
    EXPECT_THROW(seq.Get(0), IndexOutOfRange);
}

TEST(LazySequenceTest, FromArray) {
    int items[] = {10, 20, 30, 40};
    LazySequence<int> seq(items, 4);
    EXPECT_EQ(seq.GetLength(), 4);
    EXPECT_EQ(seq.Get(0), 10);
    EXPECT_EQ(seq.Get(3), 40);
    EXPECT_THROW(seq.Get(4), IndexOutOfRange);
}

TEST(LazySequenceTest, FiniteFromFunction) {
    static int x = 0;
    std::function<int()> gen = []() { return x++; };
    LazySequence<int> seq(gen, 5);
    EXPECT_TRUE(seq.IsFinite());
    EXPECT_EQ(seq.GetLength(), 5);
    EXPECT_EQ(seq.Get(0), 0);
    EXPECT_EQ(seq.Get(4), 4);
    EXPECT_THROW(seq.Get(5), IndexOutOfRange);
    x = 0;
}

TEST(LazySequenceTest, InfiniteArithmetic) {
    int idx = 0;
    std::function<int()> gen = [&idx]() { return 5 + 3 * idx++; };
    LazySequence<int> seq(gen);
    EXPECT_FALSE(seq.IsFinite());
    EXPECT_EQ(seq.GetLength(), static_cast<size_t>(-1));
    EXPECT_EQ(seq.Get(0), 5);
    EXPECT_EQ(seq.Get(1), 8);
    EXPECT_EQ(seq.Get(100), 5 + 3 * 100);
}

TEST(LazySequenceTest, Memoization) {
    int computeCount = 0;
    int idx = 0;
    std::function<int()> gen = [&computeCount, &idx]() {
        ++computeCount;
        int current = idx;
        ++idx;
        return current * current;
    };
    LazySequence<int> seq(gen);
    EXPECT_EQ(seq.Get(5), 25);
    EXPECT_EQ(seq.Get(5), 25);
    EXPECT_EQ(computeCount, 6);
}


TEST(LazySequenceTest, InsertAt) {
    int arr[] = {1,2,3};
    LazySequence<int> base(arr, 3);
    auto inserted = base.InsertAt(1, 99);
    EXPECT_EQ(inserted->GetLength(), 4);
    EXPECT_EQ(inserted->Get(0), 1);
    EXPECT_EQ(inserted->Get(1), 99);
    EXPECT_EQ(inserted->Get(2), 2);
    EXPECT_EQ(inserted->Get(3), 3);
    delete inserted;
}

TEST(LazySequenceTest, SkipFirst) {
    int arr[] = {0,1,2,3,4,5};
    LazySequence<int> seq(arr, 6);
    auto skipped = seq.SkipFirst(2);
    EXPECT_EQ(skipped->GetLength(), 4);
    EXPECT_EQ(skipped->Get(0), 2);
    EXPECT_EQ(skipped->Get(3), 5);
    delete skipped;
}

TEST(LazySequenceTest, SkipRange) {
    int arr[] = {0,1,2,3,4,5};
    LazySequence<int> seq(arr, 6);
    auto sub = seq.Skip(1, 4);
    EXPECT_EQ(sub->GetLength(), 4);
    EXPECT_EQ(sub->Get(0), 1);
    EXPECT_EQ(sub->Get(3), 4);
    delete sub;
}

TEST(LazySequenceTest, ConcatFinite) {
    int arr1[] = {1,2};
    int arr2[] = {3,4};
    LazySequence<int> seq1(arr1, 2);
    LazySequence<int> seq2(arr2, 2);
    auto concat = seq1.Concat(&seq2);
    EXPECT_EQ(concat->GetLength(), 4);
    EXPECT_EQ(concat->Get(0), 1);
    EXPECT_EQ(concat->Get(2), 3);
    EXPECT_EQ(concat->Get(3), 4);
    delete concat;
}

TEST(LazySequenceTest, ConcatInfiniteWithFinite) {
    int idx = 0;
    std::function<int()> gen = [&idx]() { return idx++; };
    LazySequence<int> infinite(gen);
    int arr[] = {-1,-2};
    LazySequence<int> finite(arr, 2);
    auto concat = infinite.Concat(&finite);
    EXPECT_FALSE(concat->IsFinite());
    EXPECT_EQ(concat->Get(0), 0);
    EXPECT_EQ(concat->Get(1000), 1000);
    EXPECT_NO_THROW(concat->Get(10000));
    delete concat;
}

TEST(LazySequenceTest, Map) {
    int arr[] = {1,2,3};
    LazySequence<int> seq(arr, 3);
    auto mapped = seq.Map<int>([](int x) { return x * 10; });
    EXPECT_EQ(mapped->GetLength(), 3);
    EXPECT_EQ(mapped->Get(0), 10);
    EXPECT_EQ(mapped->Get(2), 30);
    delete mapped;
}

TEST(LazySequenceTest, WhereFilter) {
    int arr[] = {1,2,3,4,5};
    LazySequence<int> seq(arr, 5);
    auto even = seq.Where([](int x) { return x % 2 == 0; });
    EXPECT_EQ(even->Get(0), 2);
    EXPECT_EQ(even->Get(1), 4);
    EXPECT_THROW(even->Get(2), IndexOutOfRange);  // теперь бросает IndexOutOfRange
    delete even;
}

TEST(LazySequenceTest, Reduce) {
    int arr[] = {1,2,3,4,5};
    LazySequence<int> seq(arr, 5);
    int sum = seq.Reduce([](int a, int b) { return a + b; });
    EXPECT_EQ(sum, 15);
    LazySequence<int> empty;
    EXPECT_THROW(empty.Reduce([](int a,int b){return a+b;}), EmptySequence);
    int idx = 0;
    std::function<int()> gen = [&idx](){ return idx++; };
    LazySequence<int> inf(gen);
    EXPECT_THROW(inf.Reduce([](int a,int b){return a+b;}), InfiniteSequence);
}

TEST(LazySequenceTest, MixedOperations) {
    int arr[] = {1,2,3,4,5};
    LazySequence<int> seq(arr, 5);
    auto result = seq.Where([](int x){ return x%2==0; })
                    ->Map<int>([](int x){ return x*10; })
                    ->SkipFirst(1);
    EXPECT_EQ(result->GetLength(), static_cast<size_t>(-1));
    EXPECT_EQ(result->Get(0), 40);
    delete result;
}