#include "zaberUtils.h"

#include <epicsStdio.h>

#include "zaber/motion/library.h"

namespace ze = zaber::epics;
namespace zml = zaber::motion;

ze::DeviceDbRegistrar ze::DeviceDbRegistrar::registrar;

ze::DeviceDbRegistrar::DeviceDbRegistrar() {
    const char *dbPath = std::getenv("ZABER_DEVICE_DB_PATH");
    if (!dbPath) {
        return;
    }

    try {
        zml::Library::setDeviceDbSource(zml::DeviceDbSourceType::FILE, dbPath);
    } catch (const std::runtime_error &e) {
        printf("Zaber Motion Error: Failed to set device DB source to %s\n", dbPath);
        printf("\tFailed with error: %s\n\n", e.what());
        printf("Please check that ZABER_DEVICE_DB_PATH is set to the correct db path\n");
    }
}
