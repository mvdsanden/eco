#include <net/asyncsocketmanager.h>

namespace eco {
namespace net {

// namespace {
//     struct RegistrationImpl : AsyncSocketManager::Registration {
//         AsyncSocketManager *d_manager;
//         AsyncSocket *d_socket;
//         std::mutex d_mutex;
//         size_t d_minimumReadByteCount = 0;
//         size_t d_minimumWriteByteCount = 0;
//         std::shared_ptr<AsyncContext> d_asyncReadContext;
//         std::shared_ptr<AsyncContext> d_asyncWriteContext;
//         std::vector<std::shared_ptr<Buffer>> d_readBuffers;
//         std::vector<std::shared_ptr<Buffer>> d_writeBuffers;

//         Registration(AsyncSocketManager *manager, AsyncSocket *socket)
//         : d_manager(manager)
//         , d_socket(socket)
//         {
//         }

//         ~Registration() {
//             d_manager->unregisterSocket(d_socket);
//         }
//     };
// }

// static std::shared_ptr<AsyncSocketManager> AsyncSocketManager::instance(std::unique_ptr<AsyncSocketManager> setInstance) {
//     static std::weak_ptr<AsyncSocketManager> s_instance;
//     static std::mutex s_mutex;

//     auto lock = std::lock_guard(d_mutex);

//     if (setInstance != nullptr) {
//         auto object = std::shared_ptr<AsyncSocketManager>(setInstance);
//         s_instance = object;
//         return std::move(object);
//         // TODO: clear s_instance in deleter!
//     }
    
//     auto object = s_instance.lock();
//     if (object == nullptr) {
//         auto newObject = new AsyncSocketManager();
//         s_instance = object = std::shared_ptr<AsyncSocketManager>(newObject, [](AsyncSocketManager *newObject){
//             auto lock = std::lock_guard(d_mutex);
//             // If we don't do this the weak_ptr will keep a reference to the
//             // shared_ptr data and the memory is not released, even if the
//             // object is destructed.
//             if (s_instance.get() == newObject) {
//                 s_instance.reset();
//             }
//             delete newObject;
//         });
//     }

//     return object;
// }

// void AsyncSocketManager::unregisterSocket(AsyncSocket *socket) {

// }

// std::shared_ptr<Registration> AsyncSocketManager::registerSocket(AsyncSocket *socket) {

// }

}
}