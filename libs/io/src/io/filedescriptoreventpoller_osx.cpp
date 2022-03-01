#if __APPLE__

#include <io/filedescriptoreventpoller.h>

#include <array>
#include <cassert>

#include <unistd.h>
#include <sys/event.h>

namespace eco {
namespace io {

namespace {

const size_t c_eventBufferSize = 256;

class KQueueFileDescriptorEventPollerBuffer : public FileDescriptorEventPollerBuffer {
    int d_kQueue = -1;
    std::array<struct kevent, c_eventBufferSize> d_events;
    size_t d_bufferSize = 0;
public:
    KQueueFileDescriptorEventPollerBuffer(int kQueue) : d_kQueue(kQueue) {}

    virtual FileDescriptorEvent waitForNextEvent(std::chrono::milliseconds const &timeout) noexcept override {
        if (d_bufferSize == 0) {
            if (!pollEvents(timeout)) {
                return FileDescriptorEvent{
                    nullptr,
                    -1,
                    FileDescriptorEventType::Error
                };
            }
        }
        if (d_bufferSize == 0) {
            return FileDescriptorEvent{nullptr, -1, FileDescriptorEventType::Timeout};
        }
        return kEventToFileDescriptorEvent(d_events[--d_bufferSize]);
    }

private:
    bool pollEvents(std::chrono::milliseconds timeout) {
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeout);
        timeout -= seconds;
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout);
        auto timeoutTimeSpec = timespec{seconds.count(), nanoseconds.count()};
        int count = kevent(d_kQueue, nullptr, 0, d_events.data(), c_eventBufferSize, &timeoutTimeSpec);

        if (count == -1) {
            return false;
        }

        d_bufferSize = count;

        return true;
    }

    static FileDescriptorEvent kEventToFileDescriptorEvent(struct kevent const &event) {
        return FileDescriptorEvent{
            event.udata,
            static_cast<FileDescriptor>(event.ident),
            filterToEventType(event.filter)
        };
    }

    static FileDescriptorEventType filterToEventType(int16_t filter) {
        switch (filter) {
            case EVFILT_READ: return FileDescriptorEventType::Readable;
            case EVFILT_WRITE: return FileDescriptorEventType::Writable;
            default: return FileDescriptorEventType::Error;
        }
    }
};

class KQueueFileDescriptorEventRegistry : public FileDescriptorEventRegistry {
    int d_kQueue = -1;
public:
    KQueueFileDescriptorEventRegistry() {
        d_kQueue = kqueue();
    }

    virtual ~KQueueFileDescriptorEventRegistry() {
        close(d_kQueue);
    }

    bool addOrReplace(FileDescriptor fileDescriptor, void *userData = nullptr) noexcept override
    {
        struct kevent events[2];

        EV_SET(&events[0], fileDescriptor, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, userData);
        EV_SET(&events[1], fileDescriptor, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, userData);

        return kevent(d_kQueue, events, 2, nullptr, 0, nullptr) != -1;
    }

    bool remove(FileDescriptor fileDescriptor) noexcept override
    {
        struct kevent events[2];

        EV_SET(&events[0], fileDescriptor, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        EV_SET(&events[1], fileDescriptor, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);

        return kevent(d_kQueue, events, 2, nullptr, 0, nullptr) != -1;
    }

protected:
    std::unique_ptr<FileDescriptorEventPollerBuffer> createPollerBuffer() noexcept override {
        return std::make_unique<KQueueFileDescriptorEventPollerBuffer>(d_kQueue);
    }
};

}

std::unique_ptr<FileDescriptorEventRegistry> FileDescriptorEventRegistry::createSystemDefault() noexcept {
    return std::make_unique<KQueueFileDescriptorEventRegistry>();
}

}
}

#endif
