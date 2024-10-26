// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include "zaber/motion/dto/device_db_source_type.h"
#include "zaber/motion/dto/log_output_mode.h"
#include <string>
#include <optional>


namespace zaber {
namespace motion {


/**
 * Access class to general library information and configuration.
 */
class Library {
public:
    /**
     * Sets library logging output.
     * @param mode Logging output mode.
     * @param filePath Path of the file to open.
     */
    static void setLogOutput(const LogOutputMode& mode, const std::optional<std::string>& filePath = {});

    /**
     * Sets source of Device DB data. Allows selection of a web service or a local file.
     * @param sourceType Source type.
     * @param urlOrFilePath URL of the web service or path to the local file.
     * Leave empty for the default URL of Zaber web service.
     */
    static void setDeviceDbSource(const DeviceDbSourceType& sourceType, const std::optional<std::string>& urlOrFilePath = {});

    /**
     * Enables Device DB store.
     * The store uses filesystem to save information obtained from the Device DB.
     * The stored data are later used instead of the Device DB.
     * @param storeLocation Specifies relative or absolute path of the folder used by the store.
     * If left empty defaults to a folder in user home directory.
     * Must be accessible by the process.
     */
    static void enableDeviceDbStore(const std::optional<std::string>& storeLocation = {});

    /**
     * Disables Device DB store.
     */
    static void disableDeviceDbStore();

    /**
     * Disables certain customer checks (like FF flag).
     * @param mode Whether to turn internal mode on or off.
     */
    static void setInternalMode(bool mode);

    /**
     * Sets the period between polling for IDLE during movements.
     * Caution: Setting the period too low may cause performance issues.
     * @param period Period in milliseconds.
     * Negative value restores the default period.
     */
    static void setIdlePollingPeriod(int period);

    /**
     * Throws an error if the version of the loaded shared library does not match the caller's version.
     */
    static void checkVersion();

protected:
};


}  // Namespace motion
}  // Namespace zaber
