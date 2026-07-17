#ifndef ATTACK_APPS_H
#define ATTACK_APPS_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include <vector>
#include "crypto-utils.h"

// ---------------------------------------------------------------------------
// SybilAttackApp
// Generates `numFakeIdentities` fresh ECDSA keypairs at runtime and
// self-signs a certificate for each (i.e. NOT signed by the real network
// CA), then floods the target RSU with AUTH_REQUESTs, one per fake
// identity, at a configurable rate. Demonstrates the CA-signature check as
// the primary Sybil defense; the RSU's rate limiter provides a secondary
// defense against high request rates from a single source address.
// ---------------------------------------------------------------------------
class SybilAttackApp : public ns3::Application
{
public:
  static ns3::TypeId GetTypeId ();
  SybilAttackApp ();
  void Setup (ns3::Address targetRsu, uint32_t numFakeIdentities,
              ns3::Time interval, uint32_t startingFakeNodeId = 100000);

private:
  void StartApplication () override;
  void StopApplication () override;
  void SendNextFakeIdentity ();

  ns3::Ptr<ns3::Socket> m_socket;
  ns3::Address m_target;
  uint32_t m_numFakeIdentities;
  uint32_t m_sent = 0;
  uint32_t m_startingFakeNodeId;
  ns3::Time m_interval;
  ns3::EventId m_event;
};

// ---------------------------------------------------------------------------
// ReplayAttackApp
// Holds the raw serialized bytes of a single legitimately-signed
// AUTH_REQUEST (as if captured off the wire by an eavesdropper) and resends
// those exact bytes verbatim to the target RSU at scheduled times. Because
// the RSU enforces a freshness window (timestamp) and monotonically
// increasing nonce, the replayed packet is expected to be rejected on
// every resend after the original.
// ---------------------------------------------------------------------------
class ReplayAttackApp : public ns3::Application
{
public:
  static ns3::TypeId GetTypeId ();
  ReplayAttackApp ();
  void Setup (ns3::Address targetRsu, std::vector<uint8_t> capturedPacketBytes,
              std::vector<ns3::Time> replayTimes);

private:
  void StartApplication () override;
  void StopApplication () override;
  void Replay ();

  ns3::Ptr<ns3::Socket> m_socket;
  ns3::Address m_target;
  std::vector<uint8_t> m_captured;
  std::vector<ns3::Time> m_replayTimes;
  std::vector<ns3::EventId> m_events;
};

// ---------------------------------------------------------------------------
// DosFloodAttackApp
// Sends garbage/oversized UDP packets to the target RSU's port at a high
// rate for a configured duration, to measure the effect on the RSU's
// ability to process legitimate AUTH_REQUEST / DATA_MSG traffic (queueing
// delay, rate-limiter drop rate, degraded auth latency for legitimate
// nodes sharing the channel/RSU).
// ---------------------------------------------------------------------------
class DosFloodAttackApp : public ns3::Application
{
public:
  static ns3::TypeId GetTypeId ();
  DosFloodAttackApp ();
  void Setup (ns3::Address targetRsu, ns3::Time packetInterval,
              uint32_t packetSizeBytes, ns3::Time duration);

private:
  void StartApplication () override;
  void StopApplication () override;
  void SendFloodPacket ();

  ns3::Ptr<ns3::Socket> m_socket;
  ns3::Address m_target;
  ns3::Time m_interval;
  uint32_t m_packetSize;
  ns3::Time m_duration;
  ns3::EventId m_event;
};

#endif // ATTACK_APPS_H
