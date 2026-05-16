#include <gtest/gtest.h>
#include "stream/ReadOnlyStream.hpp"
#include "stream/FileStream.hpp"
#include "lazy/LazySequence.hpp"
#include "core/Sequence.hpp"
#include <fstream>
#include <cstdio>

static void CreateTempFile(const std::string& filename, const std::vector<std::string>& lines) {
    std::ofstream file(filename);
    for (const auto& line : lines) {
        file << line << "\n";
    }
}

TEST(StreamTest, ReadFromLazySequence) {
    auto seq = std::make_unique<LazySequence<int>>(new ArraySequence<int>({10, 20, 30}));
    ReadOnlyStream<int> stream(std::move(seq));
    stream.Open();
    EXPECT_FALSE(stream.IsEndOfStream());
    EXPECT_EQ(stream.Read(), 10);
    EXPECT_EQ(stream.GetPosition(), 1);
    EXPECT_EQ(stream.Read(), 20);
    EXPECT_EQ(stream.Read(), 30);
    EXPECT_TRUE(stream.IsEndOfStream());
    EXPECT_THROW(stream.Read(), EndOfSequence);
    stream.Close();
}

TEST(StreamTest, Seek) {
    auto seq = std::make_unique<LazySequence<int>>(new ArraySequence<int>({1,2,3,4,5}));
    ReadOnlyStream<int> stream(std::move(seq));
    stream.Open();
    stream.Seek(3);
    EXPECT_EQ(stream.Read(), 4);
    stream.Seek(0);
    EXPECT_EQ(stream.Read(), 1);
    EXPECT_THROW(stream.Seek(10), IndexOutOfRange);
}

TEST(StreamTest, FileStreamRead) {
    const std::string testFile = "test_stream_temp.txt";
    CreateTempFile(testFile, {"100", "200", "300", "400"});

    FileStream<int> fileStream(testFile, Deserializer<int>());
    fileStream.Open();
    EXPECT_EQ(fileStream.Read(), 100);
    EXPECT_EQ(fileStream.Read(), 200);
    EXPECT_EQ(fileStream.Read(), 300);
    EXPECT_EQ(fileStream.Read(), 400);
    EXPECT_TRUE(fileStream.IsEndOfStream());
    fileStream.Close();
    std::remove(testFile.c_str());
}

TEST(StreamTest, FileStreamSeek) {
    const std::string testFile = "test_stream_seek.txt";
    CreateTempFile(testFile, {"a", "b", "c", "d", "e"});

    FileStream<std::string> fileStream(testFile, Deserializer<std::string>(), true);
    fileStream.Open();
    fileStream.Seek(2);
    EXPECT_EQ(fileStream.Read(), "c");
    fileStream.Seek(0);
    EXPECT_EQ(fileStream.Read(), "a");
    fileStream.Close();
    std::remove(testFile.c_str());
}

TEST(StreamTest, InfiniteStream) {
    auto infinite = std::make_unique<LazySequence<int>>([](size_t i) { return static_cast<int>(i); });
    ReadOnlyStream<int> stream(std::move(infinite));
    stream.Open();
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(stream.Read(), i);
    }
    EXPECT_FALSE(stream.IsEndOfStream());
    for (int i = 100; i < 200; ++i) {
        EXPECT_EQ(stream.Read(), i);
    }
    stream.Close();
}