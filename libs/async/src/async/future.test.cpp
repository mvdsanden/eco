#include <async/future.h>

#include <gtest/gtest.h>

using namespace eco::async;

namespace {
    const int VALUE = 1;
}

class Promise1 : public Future<int>::Promise {
    public:
        void setResultCallback(Future<int>::ResultCallback callback) noexcept override {
            callback(VALUE);
        }
};

TEST(Future, SetResultCallback) {
    // GIVEN
    auto sut = Future<int>(std::make_unique<Promise1>());

    // WHEN
    int resultValue = -1;
    sut.setResultCallback([&resultValue](int value) {
        resultValue = value;
    });

    // THEN
    EXPECT_EQ(VALUE, resultValue);
}
