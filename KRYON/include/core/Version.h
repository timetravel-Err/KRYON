#ifndef KRYON_VERSION_H
#define KRYON_VERSION_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 *
 * File: Version.h
 *
 * Description:
 * Stores framework version information.
 *
 * Version : 0.1.0
 *
 * ----------------------------------------------------------
 */

#include <string>

namespace kryon
{

class Version
{
public:

    static constexpr const char* FRAMEWORK_NAME = "KRYON";
    static constexpr const char* FRAMEWORK_VERSION = "0.1.0";
    static constexpr const char* NS3_VERSION = "3.41";

    static std::string GetFrameworkVersion()
    {
        return std::string(FRAMEWORK_NAME) +
               " v" +
               FRAMEWORK_VERSION;
    }

    static std::string GetNs3Version()
    {
        return std::string("ns-") + NS3_VERSION;
    }
};

}

#endif
