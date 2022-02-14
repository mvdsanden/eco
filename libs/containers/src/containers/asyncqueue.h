#ifndef ECO_CONTAINERS_ASYNCQUEUE
#define ECO_CONTAINERS_ASYNCQUEUE

#include <mutex>
#include <queue>
#include <condition_variable>
#include <optional>

namespace eco {
namespace containers {

    /**
     * @brief Thread-safe multiple producer multiple consumer queue.
     * 
     * All methods are safe to call concurrently from multiple threads.
     * 
     * It is guarranteed that the order in which items are consumed is in the same order
     * as they are produced.
     * 
     * @tparam T The value type of items.
     */
    template <typename T>
    class AsyncQueue {
        // PRIVATE DATA
        std::queue<T> d_queue;
        std::mutex d_mutex;
        std::condition_variable d_conditionVariable;
    public:
        // PUBLIC MANIPULATORS

        /**
         * @brief Push the specified `value` to the queue.
         * 
         * @param value 
         */
        void push(const T& value) noexcept {
            std::unique_lock<std::mutex> lock(d_mutex);
            d_queue.push(value);
            d_conditionVariable.notify_one();
        }

        /**
         * @brief Emplace a new object on the queue using the specified `args`.
         */
        template <typename... Args>
        void emplace(Args... args) noexcept {
            std::unique_lock<std::mutex> lock(d_mutex);
            d_queue.emplace(std::forward<Args...>(args...));
            d_conditionVariable.notify_all();
        }

        /**
         * @brief Swaps this queue with the specified `other` queue.
         * 
         * Note that this makes sure that if it means that a queue it non-empty after this
         * operation any blocking `pop` operation is unblocked.
         */
        void swap(AsyncQueue &other) noexcept {
            if (this == &other) {
                return;
            }

            std::scoped_lock lock(d_mutex, other.d_mutex);
            d_queue.swap(other.d_queue);
            other.d_conditionVariable.notify_all();
            d_conditionVariable.notify_all();
        }

        // PUBLIC ACCESSORS

        /**
         * @brief Block until the queue is not empty and pop the front object from the queue and
         *        return it.
         * 
         * @return T 
         */
        T pop() noexcept {
            std::unique_lock<std::mutex> lock(d_mutex);
            d_conditionVariable.wait(lock, [this](){ return !d_queue.empty(); });
            auto result = d_queue.front();
            d_queue.pop();
            return result;
        }

        /**
         * @brief If the queue is not empty pop the front object from the queue and return it,
         *        otherwise return an empty optional.
         * 
         * @return std::optional<T> 
         */
        std::optional<T> tryPop() noexcept {
            std::unique_lock<std::mutex> lock(d_mutex);
            if (d_queue.empty()) {
                return std::optional<T>();
            }

            auto result = d_queue.front();
            d_queue.pop();
            return std::optional<T>(result);
        }

        /**
         * @brief Returns the size of the queue.
         */
        size_t size() const {
            return d_queue.size();
        }

        /**
         * @brief Returns `true` if the queue is empty, `true` otherwise.
         */
        bool empty() const {
            return d_queue.empty();
        }
    };
}
}

#endif //  ECO_CONTAINERS_ASYNCQUEUE
