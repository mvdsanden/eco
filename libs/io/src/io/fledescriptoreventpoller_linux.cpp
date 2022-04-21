#if __LINUX__ || __UNIX__ || __linux__

#include <io/filedescriptoreventpoller.h>

#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include <sys/epoll.h>
#include <unistd.h>

namespace eco {
namespace io {

namespace {

const size_t c_eventBufferSize = 256;

class EventDataManager {
public:
  // PUBLIC TYPES
  using EventDataKey = uint64_t;
  using EventData =
      std::pair<FileDescriptor, std::weak_ptr<FileDescriptorEventUserData>>;

private:
  std::unordered_map<EventDataKey, std::unique_ptr<EventData>> d_eventData;
  std::unordered_map<FileDescriptor, EventDataKey> d_fileDescriptorKeys;
  EventDataKey d_eventDataKeyCounter = 0;
  std::shared_mutex d_mutex;

public:
  std::pair<EventDataKey, bool>
  addOrReplace(FileDescriptor fileDescriptor,
               std::weak_ptr<FileDescriptorEventUserData> userData) {
    std::pair<EventDataKey, bool> result(0, true);

    auto data = std::make_unique<EventData>(fileDescriptor, userData);

    auto lock = std::unique_lock<std::shared_mutex>(d_mutex);
    result.first = ++d_eventDataKeyCounter;
    d_eventData[result.first] = std::move(data);

    auto iter = d_fileDescriptorKeys.find(fileDescriptor);
    if (iter != d_fileDescriptorKeys.end()) {
      d_eventData.erase(iter->second);
      result.second = false;
    }

    d_fileDescriptorKeys[fileDescriptor] = result.first;
    return result;
  }

  EventData get(EventDataKey key) {
    auto lock = std::shared_lock(d_mutex);
    auto iter = d_eventData.find(key);
    return iter == d_eventData.end()
               ? EventData(-1, std::weak_ptr<FileDescriptorEventUserData>())
               : *(iter->second);
  }

  bool remove(FileDescriptor FileDescriptor) {
    auto lock = std::unique_lock<std::shared_mutex>(d_mutex);
    auto iter = d_fileDescriptorKeys.find(FileDescriptor);
    if (iter == d_fileDescriptorKeys.end()) {
      return false;
    }

    d_eventData.erase(iter->second);
    d_fileDescriptorKeys.erase(iter);

    return true;
  }
};

class EPollFileDescriptorEventPollerBuffer
    : public FileDescriptorEventPollerBuffer {
  int d_epoll = -1;
  EventDataManager &d_eventDataManager;
  std::array<struct epoll_event, c_eventBufferSize> d_events;
  size_t d_bufferSize = 0;

public:
  EPollFileDescriptorEventPollerBuffer(int epoll,
                                       EventDataManager &eventDataManager)
      : d_epoll(epoll), d_eventDataManager(eventDataManager) {}

  virtual FileDescriptorEvent
  waitForNextEvent(std::chrono::milliseconds const &timeout) noexcept override {
    FileDescriptorEvent result{std::weak_ptr<FileDescriptorEventUserData>(), -1,
                               FileDescriptorEventType::Error};

    if (d_bufferSize == 0) {
      if (!pollEvents(timeout)) {
        return result;
      }
    }

    if (d_bufferSize == 0) {
      result.eventType = FileDescriptorEventType::Timeout;
      return result;
    }

    auto &event = d_events[d_bufferSize - 1];

    auto eventData = d_eventDataManager.get(event.data.u64);

    result.fileDescriptor = eventData.first;
    result.userData = eventData.second;

    if ((event.events & EPOLLIN) != 0) {
      result.eventType = FileDescriptorEventType::Readable;
      event.events &= ~EPOLLIN;
    } else if ((event.events & EPOLLOUT) != 0) {
      result.eventType = FileDescriptorEventType::Writable;
      event.events &= ~EPOLLOUT;
    }

    if ((event.events & (EPOLLIN | EPOLLOUT)) == 0) {
      --d_bufferSize;
    }

    return result;
  }

private:
  bool pollEvents(std::chrono::milliseconds timeout) {
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeout);
    timeout -= seconds;
    auto nanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(timeout);
    auto timeoutTimeSpec = timespec{seconds.count(), nanoseconds.count()};

    int count = epoll_wait(d_epoll, d_events.data(), c_eventBufferSize,
                           timeout.count());

    if (count < 0) {
      return false;
    }

    d_bufferSize = count;

    return true;
  }
};

class EPollFileDescriptorEventRegistry : public FileDescriptorEventRegistry {
  // DATA
  int d_epoll = -1;
  EventDataManager d_eventDataManager;

public:
  EPollFileDescriptorEventRegistry() { d_epoll = epoll_create1(EPOLL_CLOEXEC); }

  virtual ~EPollFileDescriptorEventRegistry() { close(d_epoll); }

  bool addOrReplace(
      FileDescriptor fileDescriptor,
      std::weak_ptr<FileDescriptorEventUserData> userData =
          std::weak_ptr<FileDescriptorEventUserData>()) noexcept override {
    auto eventDataResult =
        d_eventDataManager.addOrReplace(fileDescriptor, userData);

    struct epoll_event event {
      EPOLLIN | EPOLLOUT | EPOLLET, { .u64 = eventDataResult.first }
    };

    if (eventDataResult.second) {
      return epoll_ctl(d_epoll, EPOLL_CTL_ADD, fileDescriptor, &event) != -1;
    } else {
      return epoll_ctl(d_epoll, EPOLL_CTL_MOD, fileDescriptor, &event) != -1;
    }
  }

  bool remove(FileDescriptor fileDescriptor) noexcept override {
    d_eventDataManager.remove(fileDescriptor);
    return epoll_ctl(d_epoll, EPOLL_CTL_DEL, fileDescriptor, nullptr) != -1;
  }

protected:
  std::unique_ptr<FileDescriptorEventPollerBuffer>
  createPollerBuffer() noexcept override {
    return std::make_unique<EPollFileDescriptorEventPollerBuffer>(
        d_epoll, d_eventDataManager);
  }

private:
};

} // namespace

std::unique_ptr<FileDescriptorEventRegistry>
FileDescriptorEventRegistry::createSystemDefault() noexcept {
  return std::make_unique<EPollFileDescriptorEventRegistry>();
}

} // namespace io
} // namespace eco

#endif
