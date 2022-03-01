#include <io/filedescriptoreventpoller.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sys/socket.h>
#include <fcntl.h>

using namespace eco::io;
using namespace testing;

namespace {

class MockFileDescriptorEventPollerBuffer : public FileDescriptorEventPollerBuffer {
public:
    MOCK_METHOD(FileDescriptorEvent, waitForNextEvent, (std::chrono::milliseconds const &timeout), (noexcept, override));
};

class MockFileDescriptorEventRegistry : public FileDescriptorEventRegistry {
public:
    MOCK_METHOD(bool, addOrReplace, (FileDescriptor fileDescriptor, void *userData), (noexcept, override));
    MOCK_METHOD(bool, remove, (FileDescriptor fileDescriptor), (noexcept, override));
    MOCK_METHOD(std::unique_ptr<FileDescriptorEventPollerBuffer>, createPollerBuffer, (), (noexcept, override));
};

std::pair<int, int> createNonBlockingSocketPair() {
    int sockets[2] = {-1};
    int result = socketpair(PF_LOCAL, SOCK_STREAM, 0, sockets);
    EXPECT_NE(-1, result) << strerror(errno);
    fcntl(sockets[0], F_SETFL, fcntl(sockets[0], F_GETFL) | O_NONBLOCK);
    fcntl(sockets[1], F_SETFL, fcntl(sockets[1], F_GETFL) | O_NONBLOCK);
    return std::make_pair(sockets[0], sockets[1]);
}

std::set<FileDescriptorEventType> pollEventTypes(FileDescriptorEventPoller &poller) {
    std::set<FileDescriptorEventType> eventTypes;

    while (true) {
        auto event = poller.waitForNextEvent(std::chrono::milliseconds(10));

        if (event.eventType == FileDescriptorEventType::Timeout) {
            break;
        }

        eventTypes.insert(event.eventType);
    }

    return eventTypes;
}

bool isWritable(std::set<FileDescriptorEventType> const &eventTypes) {
    return eventTypes.find(FileDescriptorEventType::Writable) != eventTypes.end();
}

bool isReadable(std::set<FileDescriptorEventType> const &eventTypes) {
    return eventTypes.find(FileDescriptorEventType::Readable) != eventTypes.end();
}

char buffer[1024];
int userData = 10;
}

TEST(FileDescriptorEventRegistry, AddAndRemove) {
    // GIVEN
    auto sockets = createNonBlockingSocketPair();
    auto sut = FileDescriptorEventRegistry::createSystemDefault();

    // WHEN-THEN remove before adding.
    EXPECT_FALSE(sut->remove(sockets.first));

    // WHEN-THEN add socket.
    EXPECT_TRUE(sut->addOrReplace(sockets.first, nullptr));

    // WHEN-THEN add socket again.
    EXPECT_TRUE(sut->addOrReplace(sockets.first, nullptr));

    // WHEN-THEN add socket again.
    EXPECT_TRUE(sut->addOrReplace(sockets.first, &sockets));

    // WHEN-THEN remove socket.
    EXPECT_TRUE(sut->remove(sockets.first));

    // WHEN-THEN remove again.
    EXPECT_FALSE(sut->remove(sockets.first));

    // WHEN-THEN
    EXPECT_FALSE(sut->addOrReplace(-1, nullptr));
}

TEST(FileDescriptorEventPoller, Creation) {
    // GIVEN
    std::shared_ptr<MockFileDescriptorEventRegistry> registry = std::make_unique<MockFileDescriptorEventRegistry>();
    auto bufferPtr = new MockFileDescriptorEventPollerBuffer();
    auto buffer = std::unique_ptr<MockFileDescriptorEventPollerBuffer>(bufferPtr);
    ON_CALL(*registry, createPollerBuffer()).WillByDefault(Return(ByMove(std::move(buffer))));

    // EXPECT
    EXPECT_CALL(*registry, createPollerBuffer());

    // WHEN
    FileDescriptorEventPoller sut(registry);

    // EXPECT
    EXPECT_CALL(*bufferPtr, waitForNextEvent(std::chrono::milliseconds(10)));

    // WHEN
    sut.waitForNextEvent(std::chrono::milliseconds(10));
}



TEST(FileDescriptorEventRegistry, EventSemantics) {
    // GIVEN
    auto sut = std::shared_ptr<FileDescriptorEventRegistry>(FileDescriptorEventRegistry::createSystemDefault());
    auto sockets = createNonBlockingSocketPair();

    FileDescriptorEventPoller poller(sut);
    sut->addOrReplace(sockets.first, &userData);

    // WHEN polled for the first time.
    auto eventTypes = pollEventTypes(poller);

    // THEN
    EXPECT_TRUE(isWritable(eventTypes));
    EXPECT_FALSE(isReadable(eventTypes));

    // WHEN polled again
    eventTypes = pollEventTypes(poller);

    // THEN events are not reported again
    EXPECT_FALSE(isWritable(eventTypes));
    EXPECT_FALSE(isReadable(eventTypes));

    // WHEN writing to the other side
    write(sockets.second, "abcdefg", 8);
    eventTypes = pollEventTypes(poller);

    // THEN socket becomes readable
    EXPECT_FALSE(isWritable(eventTypes));
    EXPECT_TRUE(isReadable(eventTypes));

    // WHEN writing a bit.
    write(sockets.first, buffer, 1);
    eventTypes = pollEventTypes(poller);

    // THEN no events are reported
    EXPECT_FALSE(isWritable(eventTypes));
    EXPECT_FALSE(isReadable(eventTypes));

    // WHEN writing until the buffer is full.
    while (write(sockets.first, buffer, 1024) != -1);
    eventTypes = pollEventTypes(poller);

    // THEN no events are reported
    EXPECT_FALSE(isWritable(eventTypes));
    EXPECT_FALSE(isReadable(eventTypes));

    // WHEN reading a bit from the other side
    read(sockets.second, buffer, 1);
    eventTypes = pollEventTypes(poller);

    // THEN it does not becomes writable yet
    // Note: this check might be platform dependent.
    EXPECT_FALSE(isWritable(eventTypes));
    EXPECT_FALSE(isReadable(eventTypes));

    // WHEN reading the full socket buffer from the other side
    while (read(sockets.second, buffer, 1024) != -1);
    eventTypes = pollEventTypes(poller);

    // THEN it becomes writable again
    EXPECT_TRUE(isWritable(eventTypes));
    EXPECT_FALSE(isReadable(eventTypes));

    // WHEN reading a bit from the socket.
    read(sockets.first, buffer, 1);
    eventTypes = pollEventTypes(poller);

    // THEN it does not become readable.
    EXPECT_FALSE(isWritable(eventTypes));
    EXPECT_FALSE(isReadable(eventTypes));

    // WHEN reading the complete buffer from the socket.
    while (read(sockets.first, buffer, 1024) != -1);
    eventTypes = pollEventTypes(poller);

    // THEN it still does not become readable.
    EXPECT_FALSE(isWritable(eventTypes));
    EXPECT_FALSE(isReadable(eventTypes));

    // WHEN writing to the other side.
    write(sockets.second, "abcdefg", 8);
    eventTypes = pollEventTypes(poller);

    // THEN it becomes readable again.
    EXPECT_FALSE(isWritable(eventTypes));
    EXPECT_TRUE(isReadable(eventTypes));
}

TEST(FileDescriptorEventRegistry, UserData) {
    // GIVEN
    auto sut = std::shared_ptr<FileDescriptorEventRegistry>(FileDescriptorEventRegistry::createSystemDefault());
    auto sockets = createNonBlockingSocketPair();
    FileDescriptorEventPoller poller(sut);

    // WHEN-THEN
    sut->addOrReplace(sockets.first, &userData);
    auto event = poller.waitForNextEvent(std::chrono::milliseconds(10));

    // THEN
    EXPECT_EQ(FileDescriptorEventType::Writable, event.eventType);
    EXPECT_EQ(&userData, event.userData);

    // WHEN
    event = poller.waitForNextEvent(std::chrono::milliseconds(10));

    // THEN
    EXPECT_EQ(FileDescriptorEventType::Timeout, event.eventType);
    EXPECT_EQ(nullptr, event.userData);
}
