#ifndef ECO_IO_FILEDESCRIPTOREVENTPOLLER
#define ECO_IO_FILEDESCRIPTOREVENTPOLLER

#include <io/filedescriptor.h>

#include <memory>
#include <chrono>

namespace eco {
namespace io {

/**
 * @brief The types of file descriptor events.
 */
enum class FileDescriptorEventType {
    /**
     * @brief An error ocurred while retrieving events.
     */
    Error = -1,

    /**
     * @brief Retrieving events timed out.
     */
    Timeout = 0,

    /**
     * @brief The file descriptor became readable.
     */
    Readable = 1,

    /**
     * @brief The file descriptor became writable.
     */
    Writable = 2
};

/**
 * @brief Describes a file descriptor event.
 */
struct FileDescriptorEvent {
    /**
     * @brief Opaque userdata pointer.
     */
    void *userData;

    /**
     * @brief The file descriptor the event ocurred on.
     */
    FileDescriptor fileDescriptor;

    /**
     * @brief The type of event that ocurred.
     */
    FileDescriptorEventType eventType;
};

/**
 * @brief This is implemented by the platform dependent poller implementation.
 */
class FileDescriptorEventPollerBuffer {
public:
    // CREATORS
    virtual ~FileDescriptorEventPollerBuffer() {}

    // MANIPULATORS
    /**
     * @brief Wait for the specified `timeout` for the next event to ocur on a registered file descriptor and return that event.
     */
    virtual FileDescriptorEvent waitForNextEvent(std::chrono::milliseconds const &timeout) noexcept = 0;
};

/**
 * @brief Registry for file descriptors that can be used to poll for events.
 * 
 * This registry is thread-safe and can be used from multiple threads concurrently.
 */
class FileDescriptorEventRegistry {
    friend class FileDescriptorEventPoller;
public:
    // STATIC CREATORS
    /**
     * @brief Create the system default file descriptor event registry and return it.
     */
    static std::unique_ptr<FileDescriptorEventRegistry> createSystemDefault() noexcept;

    virtual ~FileDescriptorEventRegistry() {}

    // MANIPULATORS
    /**
     * @brief Add or replace the specified `fileDescriptor` to the registry with the specified `userData`. Return `true` on success and `false` when `fileDescriptor` is incompatible.
     */
    virtual bool addOrReplace(FileDescriptor fileDescriptor, void *userData = nullptr) noexcept = 0;

    /**
     * @brief Remove the specified `fileDescriptor` from the registry. Return `true` on success or `false` when `fileDescriptor` was not registered.
     */
    virtual bool remove(FileDescriptor fileDescriptor) noexcept = 0;

protected:
    // PROTECTED MANIPULATORS
    virtual std::unique_ptr<FileDescriptorEventPollerBuffer> createPollerBuffer() noexcept = 0;
};

class FileDescriptorEventPoller {
    std::shared_ptr<FileDescriptorEventRegistry> d_registry;
    std::unique_ptr<FileDescriptorEventPollerBuffer> d_buffer;
public:
    // CREATORS
    FileDescriptorEventPoller(std::shared_ptr<FileDescriptorEventRegistry> registry)
    : d_registry(registry)
    , d_buffer(registry->createPollerBuffer()) {}

    // MANIPULATORS
    FileDescriptorEvent waitForNextEvent(std::chrono::milliseconds const &timeout) noexcept {
        return d_buffer->waitForNextEvent(timeout);
    }
};

}
}

#endif //  ECO_IO_FILEDESCRIPTOREVENTPOLLER
