#include "DBus.h"
#include <cstdint>
#include <iostream>
#include "unistd.h"

int main () {
    opendbus::DBusCtx ctx;
    ctx.mode = opendbus::DBUS_MODE::DBUS_PUB;
    ctx.event_num = 1;
    auto dbus = NEW_DBUS(ctx);
    // dbus->bind("ipc://0.0.0.0:12345");
    dbus->bind("ipc://ADsaabcdefg");

    int64_t cnt = 0;
    while (true) {
        opendbus::DBusData dbusData;
        dbusData.topic = "opendbus";
        dbusData.data = std::to_string(cnt++);
        dbus->send(dbusData);
        std::cout << cnt << std::endl;
        // usleep(1000 * 1000);
    }
}