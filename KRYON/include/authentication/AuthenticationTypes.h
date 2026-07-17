#ifndef KRYON_AUTHENTICATION_TYPES_H
#define KRYON_AUTHENTICATION_TYPES_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : AuthenticationTypes.h
 *
 * Description
 * -----------
 * Common authentication enumerations and definitions.
 *
 * Future authentication protocols should extend these
 * enumerations without modifying existing interfaces.
 * ----------------------------------------------------------
 */

namespace kryon
{

enum class AuthenticationMethod
{
    NONE,

    ECC,

    HASH,

    PUF,

    BLOCKCHAIN,

    DID,

    ZKP,

    POST_QUANTUM,

    CUSTOM
};
enum class AuthenticationProtocolType
{
    REFERENCE
};
enum class AuthenticationStatus
{
    NOT_STARTED,

    IN_PROGRESS,

    SUCCESS,

    FAILED
};

}

#endif
