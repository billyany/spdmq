#include "SocketFactory.hpp"
#include "EventFactory.hpp"

int main()
{
    // 创建 tcp client 对象
    auto cli = SOCK_CREATE(opendbus::DBUS_SOCKET_MODE::IPV4_TCP_CLI, "127.0.0.1:12345", 1000);

    // 创建 event 对象
    auto event_loop_ptr = EVT_CREATE(opendbus::DBUS_EVENT_MODE::EPOLL_ET, 1);
    cli->setEventLoopPtr(event_loop_ptr);
    cli->createSocket();
    cli->connect();
    event_loop_ptr->backgroundRun();

    while (true) {
        std::string data;
        std::cin >> data;
        cli->write(data);
    }
}