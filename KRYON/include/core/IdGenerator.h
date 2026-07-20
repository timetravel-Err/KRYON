#ifndef KRYON_ID_GENERATOR_H
#define KRYON_ID_GENERATOR_H

#include <string>

namespace kryon
{

class IdGenerator
{
public:

    static std::string NextRequestId()
    {
        static uint64_t counter = 1;

        return "REQ-" + std::to_string(counter++);
    }
};

}

#endif
