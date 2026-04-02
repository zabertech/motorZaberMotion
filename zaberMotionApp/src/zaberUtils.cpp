#include "zaberUtils.h"

namespace zaber {
namespace epics {

MotorZaberMotionException::MotorZaberMotionException(const std::string& message) :
    std::runtime_error(message) { }

} // epics
} // zaber