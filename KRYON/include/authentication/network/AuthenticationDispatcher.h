#ifndef KRYON_AUTHENTICATION_DISPATCHER_H
#define KRYON_AUTHENTICATION_DISPATCHER_H

#include "../packets/AuthenticationPacket.h"
#include "../packets/AuthenticationPacketType.h"

#include "../../core/Logger.h"
#include "../AuthenticationManager.h"
#include "../packets/AuthRequestPacket.h"
#include "../packets/AuthChallengePacket.h"
#include "../packets/AuthResponsePacket.h"
#include "../packets/AuthConfirmPacket.h"


namespace kryon
{


inline std::string PacketTypeToString(AuthenticationPacketType type)
{
    switch (type)
    {
        case AuthenticationPacketType::AUTH_REQUEST:
            return "AUTH_REQUEST";

        case AuthenticationPacketType::AUTH_CHALLENGE:
            return "AUTH_CHALLENGE";

        case AuthenticationPacketType::AUTH_RESPONSE:
            return "AUTH_RESPONSE";

        case AuthenticationPacketType::AUTH_CONFIRM:
            return "AUTH_CONFIRM";

        default:
            return "UNKNOWN";
    }
}


class AuthenticationDispatcher
{
public:

void SetAuthenticationManager(AuthenticationManager* manager)
{
    m_manager = manager;
}


    void Dispatch(const AuthenticationPacket& packet)
	{
		 //   Logger::Info( "[Dispatcher] Routing " + PacketTypeToString(packet.type) + " : " +  packet.requestId);

		if (!m_manager)
{
    Logger::Warning(
        "[Dispatcher] AuthenticationEngine not registered.");
    return;
}
		
		
		
		switch (packet.type)
		{
			case AuthenticationPacketType::AUTH_REQUEST:

				{
					Logger::Info(
						"[Dispatcher] Routing AUTH_REQUEST : " +
						packet.requestId);

					const AuthRequestPacket& request =
						static_cast<const AuthRequestPacket&>(packet);

					m_manager->ProcessAuthenticationRequest(request);

					break;
				}

			case AuthenticationPacketType::AUTH_CHALLENGE:
			{
				Logger::Info(
					"[Dispatcher] Routing AUTH_CHALLENGE : " +
					packet.requestId);

				const AuthChallengePacket& challenge =
					static_cast<const AuthChallengePacket&>(packet);

				m_manager->ProcessAuthenticationChallenge(challenge);

				break;
			}

			case AuthenticationPacketType::AUTH_RESPONSE:
			{
				Logger::Info(
					"[Dispatcher] Routing AUTH_RESPONSE : " +
					packet.requestId);

				const AuthResponsePacket& response =
					static_cast<const AuthResponsePacket&>(packet);

				m_manager->ProcessAuthenticationResponse(response);

				break;
			}

			case AuthenticationPacketType::AUTH_CONFIRM:
			{
				Logger::Info(
					"[Dispatcher] Routing AUTH_CONFIRM : " +
					packet.requestId);

				const AuthConfirmPacket& confirm =
					static_cast<const AuthConfirmPacket&>(packet);

				m_manager->ProcessAuthenticationConfirm(confirm);

				break;
			}

			default:

				Logger::Warning(
					"[Dispatcher] Unknown packet type.");

				break;
		}
	}
	
private:

	AuthenticationManager* m_manager = nullptr;
	//AuthenticationEngine* m_engine = nullptr;
	
};

}

#endif
