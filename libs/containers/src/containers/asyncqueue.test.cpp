#include <containers/asyncqueue.h>

#include <future>

#include <gtest/gtest.h>

using namespace eco::containers;

namespace {
    std::future<int> futurePop(AsyncQueue<int> &sut) {
        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;

        // WHEN
        auto result = std::async(std::launch::async, [&sut, &mutex, &cond, &ready](){
            {
                auto lock = std::lock_guard(mutex);
                ready = true;
                cond.notify_all();
            }
            return sut.pop();
        });

        {
            auto lock = std::unique_lock(mutex);
            cond.wait(lock, [&ready](){ return ready; });
        }

        return result;
    }
}

TEST(AsyncQueue, PushAndPop) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    sut.push(1);

    // THEN
    EXPECT_EQ(sut.pop(), 1);
}

TEST(AsyncQueue, PushAndPopOrder) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    for (int i = 0; i < 100; ++i) {
        sut.push(i);
    }

    // THEN
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(sut.pop(), i);
    }
}

TEST(AsyncQueue, EmplaceAndPop) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    sut.emplace(1);

    // THEN
    EXPECT_EQ(sut.pop(), 1);
}

TEST(AsyncQueue, EmplaceAndPopOrder) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    for (int i = 0; i < 100; ++i) {
        sut.emplace(i);
    }

    // THEN
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(sut.pop(), i);
    }
}

TEST(AsyncQueue, TryPopNoPush) {
    // GIVEN-WHEN
    AsyncQueue<int> sut;

    // THEN
    EXPECT_FALSE(sut.tryPop().has_value());
}

TEST(AsyncQueue, PushAndTryPop) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    sut.push(1);

    // THEN
    EXPECT_EQ(sut.tryPop(), 1);
}

TEST(AsyncQueue, EmplaceAndTryPop) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    sut.emplace(1);

    // THEN
    EXPECT_EQ(sut.tryPop(), 1);
}


TEST(AsyncQueue, PushAndPopAsync) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    auto result = futurePop(sut);

    // THEN
    EXPECT_EQ(result.wait_for(std::chrono::milliseconds(0)), std::future_status::timeout);

    // WHEN
    sut.push(1);

    // THEN
    EXPECT_EQ(result.wait_for(std::chrono::seconds(1)), std::future_status::ready);
}

TEST(AsyncQueue, PushTwoAndPopAsync) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    auto result1 = futurePop(sut);
    auto result2 = futurePop(sut);

    // THEN
    EXPECT_EQ(result1.wait_for(std::chrono::milliseconds(0)), std::future_status::timeout);
    EXPECT_EQ(result2.wait_for(std::chrono::milliseconds(0)), std::future_status::timeout);

    // WHEN
    sut.push(1);
    sut.push(2);

    // THEN
    EXPECT_EQ(result1.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    EXPECT_EQ(result2.wait_for(std::chrono::seconds(1)), std::future_status::ready);
}

TEST(AsyncQueue, EmplaceAndPopAsync) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    auto result = futurePop(sut);
    // THEN
    EXPECT_EQ(result.wait_for(std::chrono::milliseconds(0)), std::future_status::timeout);

    // WHEN
    sut.emplace(1);

    // THEN
    EXPECT_EQ(result.get(), 1);
}


TEST(AsyncQueue, EmplaceTwoAndPopAsync) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    auto result1 = futurePop(sut);
    auto result2 = futurePop(sut);

    // THEN
    EXPECT_EQ(result1.wait_for(std::chrono::milliseconds(0)), std::future_status::timeout);
    EXPECT_EQ(result2.wait_for(std::chrono::milliseconds(0)), std::future_status::timeout);

    // WHEN
    sut.emplace(1);
    sut.emplace(2);

    // THEN
    EXPECT_EQ(result1.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    EXPECT_EQ(result2.wait_for(std::chrono::seconds(1)), std::future_status::ready);
}

TEST(AsyncQueue, PushAndPopOrderAsync) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    auto future1 = std::async(std::launch::async, [&sut](){
        for (int i = 0; i < 100; ++i) {
            sut.push(i);
        }
        return true;
    });

    // THEN
    auto future2 = std::async(std::launch::async, [&sut](){
        for (int i = 0; i < 100; ++i) {
            EXPECT_EQ(sut.pop(), i);
        }
        return true;
    });

    future1.wait();
    future2.wait();
}

TEST(AsyncQueue, EmplaceAndPopOrderAsync) {
    // GIVEN
    AsyncQueue<int> sut;

    // WHEN
    auto future1 = std::async(std::launch::async, [&sut](){
        for (int i = 0; i < 100; ++i) {
            sut.emplace(i);
        }
        return true;
    });

    // THEN
    auto future2 = std::async(std::launch::async, [&sut](){
        for (int i = 0; i < 100; ++i) {
            EXPECT_EQ(sut.pop(), i);
        }
        return true;
    });

    future1.wait();
    future2.wait();
}

TEST(AsyncQueue, Size) {
    // GIVEN
    AsyncQueue<int> sut;

    for (int i = 0; i < 100; ++i) {
        // THEN
        EXPECT_EQ(sut.size(), i);

        // WHEN
        sut.emplace(i);
    }

    for (int i = 100; i > 0; --i) {
        // THEN
        EXPECT_EQ(sut.size(), i);

        // WHEN
        sut.pop();
    }

    // THEN
    EXPECT_EQ(sut.size(), 0);
}

TEST(AsyncQueue, Empty) {
    // GIVEN-WHEN
    AsyncQueue<int> sut;

    // THEN
    EXPECT_TRUE(sut.empty());

    // WHEN
    sut.push(1);

    // THEN
    EXPECT_FALSE(sut.empty());

    // WHEN
    sut.pop();

    // THEN
    EXPECT_TRUE(sut.empty());
}

TEST(AsyncQueue, SwapEmpty) {
    // GIVEN
    AsyncQueue<int> sut1;
    AsyncQueue<int> sut2;

    // WHEN
    sut1.swap(sut2);

    // THEN
    EXPECT_TRUE(sut1.empty());
    EXPECT_TRUE(sut2.empty());
}

TEST(AsyncQueue, SwapSingleItem) {
    // GIVEN
    AsyncQueue<int> sut1;
    AsyncQueue<int> sut2;
    sut1.push(1);

    // WHEN
    sut1.swap(sut2);

    // THEN
    EXPECT_TRUE(sut1.empty());
    EXPECT_EQ(sut2.size(), 1);
    EXPECT_EQ(1, sut2.pop());
}

TEST(AsyncQueue, SwapSingleItemReverse) {
    // GIVEN
    AsyncQueue<int> sut1;
    AsyncQueue<int> sut2;
    sut1.push(1);

    // WHEN
    sut2.swap(sut1);

    // THEN
    EXPECT_TRUE(sut1.empty());
    EXPECT_EQ(sut2.size(), 1);
    EXPECT_EQ(1, sut2.pop());
}

TEST(AsyncQueue, SwapMultipleItems) {
    // GIVEN
    AsyncQueue<int> sut1;
    AsyncQueue<int> sut2;

    for (int i = 0; i < 100; ++i) {
        sut1.push(i);
    }

    // WHEN
    sut1.swap(sut2);

    // THEN
    EXPECT_TRUE(sut1.empty());
    EXPECT_EQ(sut2.size(), 100);

    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(i, sut2.pop());
    }
}

TEST(AsyncQueue, SwapAsync) {
    // GIVEN
    AsyncQueue<int> sut1;
    AsyncQueue<int> sut2;

    // WHEN
    auto result = futurePop(sut2);
    sut1.push(1);

    // THEN
    EXPECT_EQ(result.wait_for(std::chrono::milliseconds(100)), std::future_status::timeout);

    // WHEN
    sut1.swap(sut2);

    // THEN
    EXPECT_EQ(result.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    EXPECT_EQ(1, result.get());
}
