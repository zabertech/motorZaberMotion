// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>
#include <utility>

#include "zaber/motion/ascii/connection.h"
#include "zaber/motion/ascii/device.h"
#include "zaber/motion/microscopy/illuminator.h"

namespace zaber {
namespace motion {

namespace ascii {
class Axis;
class Response;
class AxisSettings;
class AxisStorage;
class Warnings;
}

namespace microscopy {


/**
 * Use to control a channel (LED lamp) on an illuminator.
 * Requires at least Firmware 7.09.
 */
class IlluminatorChannel {
    using Axis = zaber::motion::ascii::Axis;
    using AxisSettings = zaber::motion::ascii::AxisSettings;
    using AxisStorage = zaber::motion::ascii::AxisStorage;
    using Warnings = zaber::motion::ascii::Warnings;
    using BaseConnection = zaber::motion::ascii::BaseConnection;
    using Connection = zaber::motion::ascii::Connection;
    using Device = zaber::motion::ascii::Device;
    using Response = zaber::motion::ascii::Response;
public:
  struct GenericCommandOptions {
    // Controls whether to throw an exception when the device rejects the command.
    bool checkErrors {true};
    // The timeout, in milliseconds, for a device to respond to the command.
    // Overrides the connection default request timeout.
    int timeout {0};
  };

  struct GenericCommandMultiResponseOptions {
    // Controls whether to throw an exception when a device rejects the command.
    bool checkErrors {true};
    // The timeout, in milliseconds, for a device to respond to the command.
    // Overrides the connection default request timeout.
    int timeout {0};
  };

    IlluminatorChannel(Illuminator illuminator, int channelNumber);

    IlluminatorChannel();

    /**
     * Turns this channel on.
     */
    void on();

    /**
     * Turns this channel off.
     */
    void off();

    /**
     * Turns this channel on or off.
     * @param on True to turn channel on, false to turn it off.
     */
    void setOn(bool on);

    /**
     * Checks if this channel is on.
     * @return True if channel is on, false otherwise.
     */
    bool isOn();

    /**
     * Sets channel intensity as a fraction of the maximum flux.
     * @param intensity Fraction of intensity to set (between 0 and 1).
     */
    void setIntensity(double intensity);

    /**
     * Gets the current intensity of this channel.
     * @return Current intensity as fraction of maximum flux.
     */
    double getIntensity();

    /**
     * Sends a generic ASCII command to this channel.
     * For more information refer to: [ASCII Protocol Manual](https://www.zaber.com/protocol-manual#topic_commands).
     * @param command Command and its parameters.
     * @param checkErrors Controls whether to throw an exception when the device rejects the command.
     * @param timeout The timeout, in milliseconds, for a device to respond to the command.
     * Overrides the connection default request timeout.
     * @return A response to the command.
     */
    Response genericCommand(const std::string& command, bool checkErrors = true, int timeout = 0);

    /**
     * Sends a generic ASCII command to this channel.
     * For more information refer to: [ASCII Protocol Manual](https://www.zaber.com/protocol-manual#topic_commands).
     * @param command Command and its parameters.
     * @param options A struct of type GenericCommandOptions. It has the following members:
     * * `checkErrors`: Controls whether to throw an exception when the device rejects the command.
     * * `timeout`: The timeout, in milliseconds, for a device to respond to the command.
     *   Overrides the connection default request timeout.
     * @return A response to the command.
     */
    Response genericCommand(const std::string& command, const IlluminatorChannel::GenericCommandOptions& options);

    /**
     * Sends a generic ASCII command to this channel and expects multiple responses.
     * Responses are returned in order of arrival.
     * For more information refer to: [ASCII Protocol Manual](https://www.zaber.com/protocol-manual#topic_commands).
     * @param command Command and its parameters.
     * @param checkErrors Controls whether to throw an exception when a device rejects the command.
     * @param timeout The timeout, in milliseconds, for a device to respond to the command.
     * Overrides the connection default request timeout.
     * @return All responses to the command.
     */
    std::vector<Response> genericCommandMultiResponse(const std::string& command, bool checkErrors = true, int timeout = 0);

    /**
     * Sends a generic ASCII command to this channel and expects multiple responses.
     * Responses are returned in order of arrival.
     * For more information refer to: [ASCII Protocol Manual](https://www.zaber.com/protocol-manual#topic_commands).
     * @param command Command and its parameters.
     * @param options A struct of type GenericCommandMultiResponseOptions. It has the following members:
     * * `checkErrors`: Controls whether to throw an exception when a device rejects the command.
     * * `timeout`: The timeout, in milliseconds, for a device to respond to the command.
     *   Overrides the connection default request timeout.
     * @return All responses to the command.
     */
    std::vector<Response> genericCommandMultiResponse(const std::string& command, const IlluminatorChannel::GenericCommandMultiResponseOptions& options);

    /**
     * Sends a generic ASCII command to this channel without expecting a response and without adding a message ID
     * For more information refer to: [ASCII Protocol Manual](https://www.zaber.com/protocol-manual#topic_commands).
     * @param command Command and its parameters.
     */
    void genericCommandNoResponse(const std::string& command);

    /**
     * Returns a serialization of the current channel state that can be saved and reapplied.
     * @return A serialization of the current state of the channel.
     */
    std::string getState();

    /**
     * Applies a saved state to this channel.
     * @param state The state object to apply to this channel.
     */
    void setState(const std::string& state);

    /**
     * Checks if a state can be applied to this channel.
     * This only covers exceptions that can be determined statically such as mismatches of ID or version,
     * the process of applying the state can still fail when running.
     * @param state The state object to check against.
     * @return An explanation of why this state cannot be set to this channel.
     */
    std::optional<std::string> canSetState(const std::string& state);

    /**
     * Returns a string that represents the channel.
     * @return A string that represents the channel.
     */
    std::string toString();

    /**
     * Illuminator of this channel.
     */
    Illuminator getIlluminator() const;

    /**
     * The channel number identifies the channel on the illuminator.
     */
    int getChannelNumber() const;

    /**
     * Settings and properties of this channel.
     */
    AxisSettings getSettings() const;

    /**
     * Key-value storage of this channel.
     */
    AxisStorage getStorage() const;

    /**
     * Warnings and faults of this channel.
     */
    Warnings getWarnings() const;

protected:
    Axis getAxis() const;

    Illuminator _illuminator;
    int _channelNumber;
};


}  // Namespace microscopy
}  // Namespace motion
}  // Namespace zaber
