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
		    Logger::Info(
        "[Dispatcher] Routing " +
        PacketTypeToString(packet.type) +
        " : " +
        packet.requestId);

		if (!m_manager)
		{
			Logger::Warning(
				"[Dispatcher] AuthenticationManager not registered.");
			return;
		}
		
		
		
		switch (packet.type)
		{
			case AuthenticationPacketType::AUTH_REQUEST:

				Logger::Info(
					"[Dispatcher] Routing AUTH_REQUEST : " +
					packet.requestId);

				break;

			case AuthenticationPacketType::AUTH_CHALLENGE:

				Logger::Info(
					"[Dispatcher] Routing AUTH_CHALLENGE : " +
					packet.requestId);

				break;

			case AuthenticationPacketType::AUTH_RESPONSE:

				Logger::Info(
					"[Dispatcher] Routing AUTH_RESPONSE : " +
					packet.requestId);

				break;

			case AuthenticationPacketType::AUTH_CONFIRM:

				Logger::Info(
					"[Dispatcher] Routing AUTH_CONFIRM : " +
					packet.requestId);

				break;

			default:

				Logger::Warning(
					"[Dispatcher] Unknown packet type.");

				break;
		}
	}
	
private:

	AuthenticationManager* m_manager = nullptr;
	
};

}

#endif
