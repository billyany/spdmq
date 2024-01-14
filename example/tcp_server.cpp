#include "SocketFactory.hpp"
#include "EventFactory.hpp"

int main()
{
    // 创建 tcp server 对象
    auto ser = SOCK_CREATE(opendbus::DBUS_SOCKET_MODE::IPV4_TCP_SER, "0.0.0.0:12345", 10);

    // 创建 event 对象
    auto event_loop_ptr = EVT_CREATE(opendbus::DBUS_EVENT_MODE::EPOLL_ET, 10);
    ser->setEventLoopPtr(event_loop_ptr);
    ser->createSocket();
    ser->bind();
    event_loop_ptr->backgroundRun();

    while (true) {
        sleep(3600);
    }
}