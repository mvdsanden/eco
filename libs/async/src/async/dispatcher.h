#ifndef ECO_ASYNC_DISPATCHER
#define ECO_ASYNC_DISPATCHER

#include <functional>

namespace eco {
namespace async {

/**
 * @brief Dispatches functions
 * 
 * Implementation of this class dispatches function either to another thread,
 * immediately executes the function or defers function execution to a later
 * time, depending on the implementation.
 */
class Dispatcher {
    public:
    using DispatchFunction = std::function<void()>;

    /**
     * @brief Dispatches the specified `function`.
     * 
     * How `function` is dispatched depends on the implementation of the dispatcher.
     */
    virtual void dispatch(DispatchFunction function) = 0;
};

}
}

#endif // #ifndef ECO_ASYNC_DISPATCHER