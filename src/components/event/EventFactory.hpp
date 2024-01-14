#pragma once

#include "EventPoll.hpp"
#include <memory>

#define EVT_CREATE opendbus::EventFactory::getInstance()->createEventLoop

namespace opendbus {

const std::map<DBUS_EVENT_MODE, int32_t> gDBusEventModeMap = {
    {DBUS_EVENT_MODE::EPOLL_LT, EPOLLIN},
    {DBUS_EVENT_MODE::EPOLL_ET, EPOLLIN | EPOLLET},
};

class EventFactory {
public:
    static EventFactory* getInstance() {
        static EventFactory impl;
        return &impl;
    }

    std::shared_ptr<EventLoop> createEventLoop(DBUS_EVENT_MODE mode, int32_t event_num) {
        std::shared_ptr<EventLoop> event_loop;
        switch (mode) {
            case DBUS_EVENT_MODE::EPOLL_LT:
            case DBUS_EVENT_MODE::EPOLL_ET:
                event_loop = std::make_shared<EventPoll>(gDBusEventModeMap.at(mode), event_num);
                break;
        }
        return event_loop;
    }
};

} /* opendbus */