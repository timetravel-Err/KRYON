#ifndef MOBILE_NODE_APP_H
#define MOBILE_NODE_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include "blockchain.h"
#include "crypto-utils.h"

// ---------------------------------------------------------------------------
// MobileNodeApp: shared NS-3 Application for both vehicles and drones (the
// authentication + data-plane protocol is identical; only the nodeType
// string and telemetry payload semantics differ, which is enough to report
// separate per-class results in the paper by filtering metrics CSVs on
// nodeType).
//
// Lifecycle:
//   1. Sends ECDSA-signed AUTH_REQUEST (carrying its CA certificate) to its
//      assigned home RSU.
//   2. Retries with fresh nonce/timestamp on AUTH_REJECT or timeout
//      (bounded, exponential-ish backoff).
//   3. Once AUTH_ACCEPT is received, derives the session token locally
//      (HMAC over the committed block hash) and begins sending periodic
//      HMAC-tagged DATA_MSG telemetry to the RSU.
// ---------------------------------------------------------------------------
class MobileNodeApp : public ns3::Application
{
public:
  static ns3::TypeId GetTypeId ();
  MobileNodeApp ();
  ~MobileNodeApp () override;

  void Setup (uint32_t nodeId, std::string nodeType, CryptoUtils::KeyPair keyPair,
              Certificate cert, ns3::Address rsuAddress,
              ns3::Time dataInterval = ns3::Seconds (1.0));

protected:
  void DoDispose () override;

private:
  void StartApplication () override;
  void StopApplication () override;

  void HandleRead (ns3::Ptr<ns3::Socket> socket);
  void SendAuthRequest ();
  void ScheduleRetry ();
  void SendDataMsg ();

  uint32_t m_nodeId;
  std::string m_nodeType;
  CryptoUtils::KeyPair m_keyPair;
  Certificate m_cert;
  ns3::Address m_rsuAddress;
  ns3::Ptr<ns3::Socket> m_socket;

  bool m_authenticated = false;
  uint64_t m_nonce = 1;
  std::vector<uint8_t> m_sessionToken;
  uint64_t m_committedBlockIndex = 0;

  ns3::EventId m_retryEvent;
  ns3::EventId m_dataEvent;
  ns3::Time m_dataInterval;
  uint32_t m_authAttempts = 0;
};

#endif // MOBILE_NODE_APP_H
