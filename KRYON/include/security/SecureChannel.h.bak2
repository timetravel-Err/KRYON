#ifndef KRYON_SECURE_CHANNEL_H
#define KRYON_SECURE_CHANNEL_H

/**
 * ----------------------------------------------------------
 * KRYON Research Framework
 * ----------------------------------------------------------
 * File : SecureChannel.h
 *
 * Description
 * -----------
 * Provides authenticated secure communication using
 * session keys established by the Authentication Engine.
 *
 * Current implementation:
 * • AES-256-GCM encryption
 * • AES-256-GCM authentication tag
 * • Per-packet random nonce
 * • Session identifier binding
 * • Basic sequence-number support
 * ----------------------------------------------------------
 */

#include "../crypto/CryptoEngine.h"
#include "../authentication/Session.h"
#include "../core/Logger.h"
#include "SecurePacket.h"
#include "../authentication/SessionManager.h"
#include <stdexcept>
#include <string>

namespace kryon
{

class SecureChannel
{
public:

    SecureChannel() = default;

    void SetCryptoEngine(CryptoEngine* crypto)
    {
        m_crypto = crypto;
    }
	
	void SetSessionManager(SessionManager* sessionManager)
	{
		m_sessionManager = sessionManager;
	}

    void Initialize()
    {
        if (m_crypto == nullptr)
        {
            throw std::runtime_error(
                "SecureChannel requires a CryptoEngine.");
        }
		if (m_sessionManager == nullptr)
		{
			throw std::runtime_error(
				"SecureChannel requires a SessionManager.");
		}

        Logger::Info(
            "Secure Channel initialized.");
    }

    void Finalize()
    {
        Logger::Info(
            "Secure Channel finalized.");
    }

    /*
     * ------------------------------------------------------
     * AES-256-GCM Encryption
     * ------------------------------------------------------
     */

    SecurePacket Encrypt(
        const ByteArray& plaintext,
        const Session& session)
    {
        if (m_crypto == nullptr)
        {
            throw std::runtime_error(
                "SecureChannel CryptoEngine not initialized.");
        }

        if (!session.active)
        {
            throw std::runtime_error(
                "Cannot encrypt using inactive session.");
        }

        if (session.sessionKey.bytes.data.size() != 32)
        {
            throw std::runtime_error(
                "SecureChannel requires a 32-byte AES-256 session key.");
        }

        SecurePacket packet;

        /*
         * Session metadata.
         */
        packet.sessionId = session.sessionId;

        packet.timestamp =
            ns3::Simulator::Now().GetSeconds();

        /*
		 * Allocate the next authenticated sequence number.
		 */
		packet.sequenceNumber =
			m_sessionManager->GetNextSendSequence(
				session.sessionId,
				packet.timestamp);

       /*
		 * Authenticate the session identifier and
		 * sequence number using AES-GCM AAD.
		 *
		 * These fields remain visible in the packet,
		 * but any modification invalidates the GCM tag.
		 */
		ByteArray aad;

		std::string aadString =
			session.sessionId +
			"|" +
			std::to_string(packet.sequenceNumber);

		aad.data.assign(
			aadString.begin(),
			aadString.end());

		/*
		 * Encrypt the application payload.
		 */
		Ciphertext encrypted =
			m_crypto->Encrypt(
				plaintext,
				session.sessionKey,
				aad);

        /*
         * Copy AES-GCM output into SecurePacket.
         */
        packet.ciphertext =
            encrypted.bytes;

        packet.nonce =
            encrypted.nonce.bytes;

        packet.mac =
            encrypted.tag.bytes;

        packet.encrypted = true;

        Logger::Info(
            "SecureChannel: AES-256-GCM encryption successful.");

        return packet;
    }

    /*
     * ------------------------------------------------------
     * AES-256-GCM Decryption
     * ------------------------------------------------------
     */

    ByteArray Decrypt(
        const SecurePacket& packet,
        const Session& session)
    {
        if (m_crypto == nullptr)
        {
            throw std::runtime_error(
                "SecureChannel CryptoEngine not initialized.");
        }

        if (!session.active)
        {
            throw std::runtime_error(
                "Cannot decrypt using inactive session.");
        }

        if (packet.sessionId != session.sessionId)
        {
            throw std::runtime_error(
                "SecureChannel session identifier mismatch.");
        }

        if (!packet.encrypted)
        {
            throw std::runtime_error(
                "SecureChannel received unencrypted packet.");
        }

        if (session.sessionKey.bytes.data.size() != 32)
        {
            throw std::runtime_error(
                "SecureChannel requires a 32-byte AES-256 session key.");
        }

        /*
         * Reconstruct the CryptoEngine Ciphertext object.
         */
        Ciphertext encrypted;

        encrypted.bytes =
            packet.ciphertext;

        encrypted.nonce.bytes =
            packet.nonce;

        encrypted.tag.bytes =
            packet.mac;

        /*
		 * Reconstruct the same authenticated data used
		 * during encryption.
		 *
		 * The sequence number is deliberately included
		 * in the AAD so that it cannot be modified without
		 * invalidating the GCM authentication tag.
		 */
		ByteArray aad;

		std::string aadString =
			packet.sessionId +
			"|" +
			std::to_string(packet.sequenceNumber);

		aad.data.assign(
			aadString.begin(),
			aadString.end());

		/*
		 * AES-256-GCM authentication/decryption.
		 *
		 * IMPORTANT:
		 * Replay state is NOT updated until this
		 * cryptographic authentication succeeds.
		 */
		ByteArray plaintext =
			m_crypto->Decrypt(
				encrypted,
				session.sessionKey,
				aad);

		/*
		 * ------------------------------------------------------
		 * Replay protection
		 * ------------------------------------------------------
		 *
		 * Only update replay state after successful
		 * cryptographic authentication.
		 */
		if (!m_sessionManager->AcceptReceivedSequence(
				packet.sessionId,
				packet.sequenceNumber,
				packet.timestamp))
		{
			throw std::runtime_error(
				"SecureChannel replay protection rejected "
				"duplicate or out-of-order packet.");
		}

				Logger::Info(
					"SecureChannel: AES-256-GCM decryption successful.");

				return plaintext;
			}

    /*
     * ------------------------------------------------------
     * MAC Compatibility Interface
     * ------------------------------------------------------
     *
     * AES-GCM already provides authenticated encryption.
     * Therefore no separate HMAC is required here.
     *
     * The GCM authentication tag is stored in
     * SecurePacket::mac.
     * ------------------------------------------------------
     */

    ByteArray ComputeMAC(
        const ByteArray&,
        const Session&)
    {
        Logger::Info(
            "SecureChannel: GCM authentication tag is used; "
            "separate MAC not required.");

        return ByteArray();
    }

    bool VerifyMAC(
        const SecurePacket&,
        const Session&)
    {
        Logger::Info(
            "SecureChannel: GCM authentication tag is verified "
            "during decryption.");

        return true;
    }

private:

    CryptoEngine* m_crypto = nullptr;
	
	SessionManager* m_sessionManager = nullptr;
};

}

#endif // KRYON_SECURE_CHANNEL_H