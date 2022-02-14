#include <async/loopdispatcher.h>

#include <future>

#include <gtest/gtest.h>

using namespace eco::async;

TEST(LoopDispatcher, DispatchOne) {
    // GIVEN
    LoopDispatcher sut;
    bool result = false;
    auto function = [&result](){ result = true; };

    // WHEN
    sut.dispatch(function);

    // THEN
    EXPECT_FALSE(result);

    // WHEN
    sut.getNextDispatch()();
    
    // THEN
    EXPECT_TRUE(result);
}

TEST(LoopDispatcher, DispatchSingleConsumerSingleProducerThreads) {
    // GIVEN
    LoopDispatcher sut;
    size_t count = 0;
    auto function = [&count](){ count++; };

    // WHEN
    auto future1 = std::async(std::launch::async, [&sut, function]{
        for (size_t i = 0; i < 100; ++i) {
            sut.dispatch(function);
        }
    });

    for (size_t i = 0; i < 100; ++i) {
        sut.getNextDispatch()();
    }
    
    // THEN
    EXPECT_EQ(count, 100);
}

TEST(LoopDispatcher, DispatchSingleConsumerMultipleProducerThreads) {
    // GIVEN
    LoopDispatcher sut;
    size_t count = 0;
    auto function = [&count](){ count++; };

    // WHEN
    std::future<void> futures[10];
    for (size_t i = 0; i < 10; ++i) {
        futures[i] = std::async(std::launch::async, [&sut, function]{
            sut.dispatch(function);
        });
    }

    for (size_t i = 0; i < 10; ++i) {
        sut.getNextDispatch()();
    }
    
    // THEN
    EXPECT_EQ(count, 10);
}

TEST(LoopDispatcher, DispatchMultipleConsumerSingleProducerThreads) {
    // GIVEN
    LoopDispatcher sut;
    std::atomic<size_t> count = 0;
    auto function = [&count](){ count++; };

    // WHEN
    auto future1 = std::async(std::launch::async, [&sut, function]{
        for (size_t i = 0; i < 100; ++i) {
            sut.dispatch(function);
        }
    });

    std::future<void> futures[10];
    for (size_t i = 0; i < 10; ++i) {
        futures[i] = std::async(std::launch::async, [&sut]{
            for (size_t i = 0; i < 10; ++i) {
                sut.getNextDispatch()();
            }
        });
    }
    
    for (size_t i = 0; i < 10; ++i) {
        futures[i].wait();
    }

    // THEN
    EXPECT_EQ(count, 100);
}