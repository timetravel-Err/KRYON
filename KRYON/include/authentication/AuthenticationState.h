#ifndef KRYON_AUTHENTICATION_STATE_H
#define KRYON_AUTHENTICATION_STATE_H

namespace kryon
{

enum class AuthenticationState
{
    IDLE,

    MESSAGE1_SENT,

    MESSAGE2_RECEIVED,

    MESSAGE3_SENT,

    VERIFYING,

    KEY_AGREEMENT,

    SESSION_ESTABLISHED,

    FAILED
};

}

#endif
