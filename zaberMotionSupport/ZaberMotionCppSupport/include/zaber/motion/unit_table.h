// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include "zaber/motion/units.h"
#include <string>
#include <vector>


namespace zaber {
namespace motion {

/**
 * Helper for working with units of measure.
 */
class UnitTable {
public:
    UnitTable(const UnitTable& other) = delete;
    UnitTable& operator=(const UnitTable& other) = delete;

    /**
     * Gets the standard symbol associated with a given unit.
     * @param unit Unit of measure.
     * @return Symbols corresponding to the given unit. Throws NoValueForKey if no symbol is defined.
     */
    static std::string getSymbol(Units unit);

    /**
     * Gets the unit enum value associated with a standard symbol.
     * Note not all units can be retrieved this way.
     * @param symbol Symbol to look up.
     * @return The unit enum value with the given symbols. Throws NoValueForKey if the symbol is not supported for lookup.
     */
    static Units getUnit(const std::string& symbol);

protected:
};


}  // Namespace motion
}  // Namespace zaber
