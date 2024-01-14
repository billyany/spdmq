#pragma once

#include "DBusPubImpl.hpp"
#include "DBusSubImpl.hpp"
#include <memory>
#include <sys/types.h>

#define DBUS_MODE_CREATE opendbus::DBusFactory::getInstance()->createDBusMode

namespace opendbus {

class DBusFactory {
public:
    static DBusFactory* getInstance() {
        static DBusFactory impl;
        return &impl;
    }

    std::shared_ptr<DBusBase> createDBusMode(const DBusCtx& ctx, DBUS_NET_PROTOCOL protocol) {
        std::shared_ptr<DBusBase> impl;
        switch (ctx.mode) {
            case DBUS_MODE::DBUS_PUB:
                impl = std::make_unique<DBusPubImpl>(ctx, protocol);
                break;
            case DBUS_MODE::DBUS_SUB:
                impl = std::make_unique<DBusSubImpl>(ctx, protocol);
                break;
        }

        return impl;
    }

protected:
    DBusFactory() {}
    ~DBusFactory() {}
};

} // namespace hh_v2x_ipc
