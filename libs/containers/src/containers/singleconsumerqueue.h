#ifndef ECO_CONTAINERS_SINGLECONSUMERQUEUE
#define ECO_CONTAINERS_SINGLECONSUMERQUEUE

#include <containers/asyncqueue.h>

namespace eco {
namespace containers {
    template <typename T>
    using SingleConsumerQueue = AsyncQueue<T>;
}
}

#endif //  ECO_CONTAINERS_SINGLECONSUMERQUEUE
