#ifndef ECO_CONTAINERS_SINGLECONSUMERQUEUE
#define ECO_CONTAINERS_SINGLECONSUMERQUEUE

#include <containers/asyncqueue.h>

namespace eco {
namespace containers {
    /**
     * @brief Single consumer queue
     * 
     * This is a queue to which multiple threads can push concurrently, but is only safe to
     * pop from one thread at a time.
     * 
     * Currently this uses the `AsyncQueue`, which is a multiple consumer, multiple producer
     * queue and therefor has stronger requirements than a single consumer queue. In the future
     * an optimized single consumer queue might be provided here.
     * 
     * TODO: Encapsulate the AsyncQueue instead of typedef so we can provide a correct contract
     *       for this type.
     */
    template <typename T>
    using SingleConsumerQueue = AsyncQueue<T>;
}
}

#endif //  ECO_CONTAINERS_SINGLECONSUMERQUEUE
