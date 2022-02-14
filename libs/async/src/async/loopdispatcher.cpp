#include <async/loopdispatcher.h>

namespace eco {
namespace async {

// PUBLIC MANIPULATORS

Dispatcher::DispatchFunction LoopDispatcher::getNextDispatch() {
    return d_queue.pop();
}

void LoopDispatcher::dispatch(DispatchFunction function) {
    d_queue.emplace(std::move(function));
}

}
}
