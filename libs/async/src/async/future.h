#ifndef ECO_ASYNC_FUTURE
#define ECO_ASYNC_FUTURE

#include <memory>
#include <functional>

namespace eco {
namespace async {
    template <typename T>
    class Future {
    public:
        using ResultCallback = std::function<void(T)>;
        class Promise {
        public:
            virtual ~Promise() {}
            virtual void setResultCallback(ResultCallback callback) noexcept = 0;
        };

    private:
        std::unique_ptr<Promise> d_promise;

    public:
        Future(std::unique_ptr<Promise> promise) : d_promise(std::move(promise)) {}

        void setResultCallback(ResultCallback callback) noexcept {
            d_promise->setResultCallback(callback);
        }
    };
}
}

#endif //  ECO_ASYNC_FUTURE