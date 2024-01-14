#include "DBus.h"
#include <iostream>

int main (int argc, char** argv) {
    opendbus::DBusCtx ctx;
    ctx.mode = opendbus::DBUS_MODE::DBUS_SUB;
    ctx.topics = {"opendbus"};
    auto dbus = NEW_DBUS(ctx);
    // dbus->connect("tcp://0.0.0.0:12345");
    dbus->connect("ipc://ADsaabcdefg");
    while (true) {
        opendbus::DBusData dbusData;
        dbus->recv(dbusData, 0);
        std::cout << dbusData.data << std::endl;
    }
}