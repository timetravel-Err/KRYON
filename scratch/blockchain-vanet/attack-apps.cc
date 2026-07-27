#include "attack-apps.h"
#include "messages.h"
#include "metrics.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/udp-socket-factory.h"

using namespace ns3;

// ============================== Sybil =====================================
NS_OBJECT_ENSURE_REGISTERED (SybilAttackApp);

TypeId
SybilAttackApp::GetTypeId ()
{
  static TypeId tid = TypeId ("SybilAttackApp").SetParent<Application> ()
    .AddConstructor<SybilAttackApp> ();
  return tid;
}

SybilAttackApp::SybilAttackApp () : m_numFakeIdentities (20), m_startingFakeNodeId (100000),
                                     m_interval (Seconds (0.1)) {}

void
SybilAttackApp::Setup (Address targetRsu, uint32_t numFakeIdentities, Time interval,
                        uint32_t startingFakeNodeId)
{
  m_target = targetRsu; m_numFakeIdentities = numFakeIdentities; m_interval = interval;
  m_startingFakeNodeId = startingFakeNodeId;
}

void
SybilAttackApp::StartApplication ()
{
  m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
  m_socket->Bind ();
  m_event = Simulator::Schedule (Seconds (0.0), &SybilAttackApp::SendNextFakeIdentity, this);
}

void
SybilAttackApp::StopApplication ()
{
  if (m_event.IsRunning ()) Simulator::Cancel (m_event);
  if (m_socket) m_socket->Close ();
}

void
SybilAttackApp::SendNextFakeIdentity ()
{
  if (m_sent >= m_numFakeIdentities) return;
  uint32_t fakeNodeId = m_startingFakeNodeId + m_sent;
  double now = Simulator::Now ().GetSeconds ();

  // Generate a throwaway keypair and a SELF-signed certificate (i.e. signed
  // with the attacker's own throwaway key, not the network CA's private
  // key). A correctly-implemented RSU will reject this at the certificate
  // verification step, since it checks the CA's signature specifically.
  CryptoUtils::KeyPair fakeKp = CryptoUtils::GenerateKeyPair ();
  Certificate cert;
  cert.nodeId = fakeNodeId; cert.nodeType = "vehicle";
  cert.publicKey = CryptoUtils::ExportPublicKey (fakeKp);
  cert.issuedAt = now; cert.expiresAt = now + 3600;
  cert.caSignature = CryptoUtils::Sign (fakeKp, cert.SerializeForSigning ()); // self-signed, not CA

  AuthRequestMsg msg;
  msg.nodeId = fakeNodeId; msg.nodeType = "vehicle"; msg.timestamp = now; msg.nonce = 1;
  ByteWriter w; w.U32 (msg.nodeId); w.Str (msg.nodeType); w.Dbl (msg.timestamp); w.U64 (msg.nonce);
  msg.signature = CryptoUtils::Sign (fakeKp, w.buf);
  msg.cert = cert;

  auto bytes = msg.Serialize ();
  Ptr<Packet> p = Create<Packet> (bytes.data (), bytes.size ());
  m_socket->SendTo (p, 0, m_target);

  MetricsCollector::Get ().LogAttackEvent (now, "SybilAttack_attempt", fakeNodeId, "sent");

  CryptoUtils::FreeKeyPair (fakeKp);
  m_sent++;
  m_event = Simulator::Schedule (m_interval, &SybilAttackApp::SendNextFakeIdentity, this);
}

// ============================== Replay =====================================
NS_OBJECT_ENSURE_REGISTERED (ReplayAttackApp);

TypeId
ReplayAttackApp::GetTypeId ()
{
  static TypeId tid = TypeId ("ReplayAttackApp").SetParent<Application> ()
    .AddConstructor<ReplayAttackApp> ();
  return tid;
}

ReplayAttackApp::ReplayAttackApp () {}

void
ReplayAttackApp::Setup (Address targetRsu, std::vector<uint8_t> capturedPacketBytes,
                         std::vector<Time> replayTimes)
{
  m_target = targetRsu; m_captured = std::move (capturedPacketBytes); m_replayTimes = replayTimes;
}

void
ReplayAttackApp::StartApplication ()
{
  m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
  m_socket->Bind ();
  for (auto &t : m_replayTimes)
    m_events.push_back (Simulator::Schedule (t, &ReplayAttackApp::Replay, this));
}

void
ReplayAttackApp::StopApplication ()
{
  for (auto &e : m_events) if (e.IsRunning ()) Simulator::Cancel (e);
  if (m_socket) m_socket->Close ();
}

void
ReplayAttackApp::Replay ()
{
  if (m_captured.empty ()) return;
  Ptr<Packet> p = Create<Packet> (m_captured.data (), m_captured.size ());
  m_socket->SendTo (p, 0, m_target);
  MetricsCollector::Get ().LogAttackEvent (Simulator::Now ().GetSeconds (),
                                            "ReplayAttack_attempt", 0, "sent");
}

// ============================== DoS Flood ===================================
NS_OBJECT_ENSURE_REGISTERED (DosFloodAttackApp);

TypeId
DosFloodAttackApp::GetTypeId ()
{
  static TypeId tid = TypeId ("DosFloodAttackApp").SetParent<Application> ()
    .AddConstructor<DosFloodAttackApp> ();
  return tid;
}

DosFloodAttackApp::DosFloodAttackApp () : m_interval (MilliSeconds (5)), m_packetSize (256),
                                           m_duration (Seconds (5.0)) {}

void
DosFloodAttackApp::Setup (Address targetRsu, Time packetInterval, uint32_t packetSizeBytes,
                           Time duration)
{
  m_target = targetRsu; m_interval = packetInterval; m_packetSize = packetSizeBytes;
  m_duration = duration;
}

void
DosFloodAttackApp::StartApplication ()
{
  m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
  m_socket->Bind ();
  m_stopAt = Simulator::Now () + m_duration; // absolute cutoff, not the raw duration itself
  m_event = Simulator::Schedule (Seconds (0.0), &DosFloodAttackApp::SendFloodPacket, this);
}

void
DosFloodAttackApp::StopApplication ()
{
  if (m_event.IsRunning ()) Simulator::Cancel (m_event);
  if (m_socket) m_socket->Close ();
}

void
DosFloodAttackApp::SendFloodPacket ()
{
  if (Simulator::Now () > m_stopAt) return;
  std::vector<uint8_t> buf (m_packetSize, 0xAA); // garbage payload, no valid message type
  Ptr<Packet> p = Create<Packet> (buf.data (), buf.size ());
  m_socket->SendTo (p, 0, m_target);
  MetricsCollector::Get ().LogAttackEvent (Simulator::Now ().GetSeconds (), "DosFlood", 0, "sent");
  m_event = Simulator::Schedule (m_interval, &DosFloodAttackApp::SendFloodPacket, this);
}
