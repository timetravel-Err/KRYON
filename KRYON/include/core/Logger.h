#ifndef KRYON_LOGGER_H
#define KRYON_LOGGER_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 *
 * File: Logger.h
 *
 * Description:
 * Simple console logger for KRYON.
 *
 * Version : 0.1.0
 * ----------------------------------------------------------
 */

#include <iostream>
#include <string>

namespace kryon
{

class Logger
{
public:

    static void Info(const std::string& message)
    {
        std::cout << "[INFO] " << message << std::endl;
    }

    static void Warning(const std::string& message)
    {
        std::cout << "[WARNING] " << message << std::endl;
    }

    static void Error(const std::string& message)
    {
        std::cerr << "[ERROR] " << message << std::endl;
    }

    static void Debug(const std::string& message)
    {
        std::cout << "[DEBUG] " << message << std::endl;
    }
};

} // namespace kryon

#endif // KRYON_LOGGER_H
