#ifndef ECO_ASYNC_ASYNCSOCKETMANAGER
#define ECO_ASYNC_ASYNCSOCKETMANAGER

#include <memory>
#include <mutex>

namespace eco {
namespace async {
// class AsyncSocketManager {
// public:
//     class AsyncSocket {
//     public:
//         virtual int socketFileDescriptor() noexcept = 0;
//         virtual void onReadCompleted() noexcept = 0;
//         virtual void onWriteCompleted() noexcept = 0;
//         // TODO: onConnectionAccepted() ?
//     };

//     class Registration {
//     public:
//         virtual ~Registration() {}
//         virtual void setWriteOperation(std::vector<std::shared_ptr<Buffer> buffers, size_t mimimumBytes, std::shared_ptr<AsyncContext> asyncContext);
//         virtual void setReadOperation(std::vector<std::shared_ptr<Buffer> buffers, size_t mimimumBytes, std::shared_ptr<AsyncContext> asyncContext);
//     };
// private:

//     void unregisterSocket(AsyncSocket *socket);

// public:

//     static std::shared_ptr<AsyncSocketManager> instance(std::unique_ptr<AsyncSocketManager> setInstance = nullptr);

//     std::shared_ptr<Registration> registerSocket(AsyncSocket *socket);
    

// };
}
}

#endif //  ECO_ASYNC_ASYNCSOCKETMANAGER