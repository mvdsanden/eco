#ifndef ECO_ASYNC_LOOPDISPATCHER
#define ECO_ASYNC_LOOPDISPATCHER

#include <async/dispatcher.h>
#include <containers/singleconsumerqueue.h>

namespace eco {
namespace async {

/**
 * @brief Dispatcher which does not have its own thread and is used inside a loop.
 * 
 */
class LoopDispatcher : public Dispatcher {

    // PRIVATE DATA
    containers::SingleConsumerQueue<DispatchFunction> d_queue;

public:

    // PUBLIC MANIPULATORS
    
    /**
     * @brief Blocks until the next function is available for dispatch and returns it.
     */
    DispatchFunction getNextDispatch();

    void dispatch(DispatchFunction function) override;
};

}
}

#endif // ECO_ASYNC_LOOPDISPATCHER
