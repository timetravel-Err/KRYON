#include "mobile-node-app.h"
#include "messages.h"
#include "metrics.h"
#include <iostream>
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/inet-socket-address.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/double.h"
#include "ns3/udp-socket-factory.h"

NS_LOG_COMPONENT_DEFINE ("MobileNodeApp");

using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED (MobileNodeApp);

TypeId
MobileNodeApp::GetTypeId ()
{
  static TypeId tid = TypeId ("MobileNodeApp")
    .SetParent<Application> ()
    .AddConstructor<MobileNodeApp> ();
  return tid;
}

MobileNodeApp::MobileNodeApp () : m_nodeId (0), m_dataInterval (Seconds (1.0)) {}
MobileNodeApp::~MobileNodeApp () {}

void
MobileNodeApp::Setup (uint32_t nodeId, std::string nodeType, CryptoUtils::KeyPair keyPair,
                       Certificate cert, Address rsuAddress, Time dataInterval)
{
  m_nodeId = nodeId; m_nodeType = std::move (nodeType); m_keyPair = keyPair;
  m_cert = std::move (cert); m_rsuAddress = rsuAddress; m_dataInterval = dataInterval;
}

void
MobileNodeApp::DoDispose ()
{
  m_socket = nullptr;
  Application::DoDispose ();
}

void
MobileNodeApp::StartApplication ()
{
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
      m_socket->Bind ();
      m_socket->SetRecvCallback (MakeCallback (&MobileNodeApp::HandleRead, this));
    }
  Ptr<UniformRandomVariable> jitter = CreateObject<UniformRandomVariable> ();
  jitter->SetAttribute ("Min", DoubleValue (0.0));
  jitter->SetAttribute ("Max", DoubleValue (0.3));
  Simulator::Schedule (Seconds (jitter->GetValue ()), &MobileNodeApp::SendAuthRequest, this);
}

void
MobileNodeApp::StopApplication ()
{
  if (m_retryEvent.IsRunning ()) Simulator::Cancel (m_retryEvent);
  if (m_dataEvent.IsRunning ()) Simulator::Cancel (m_dataEvent);
  if (m_socket) m_socket->Close ();
}

void
MobileNodeApp::SendAuthRequest ()
{
  if (m_authenticated) return;
  m_authAttempts++;
  double now = Simulator::Now ().GetSeconds ();

  AuthRequestMsg msg;
  msg.nodeId = m_nodeId; msg.nodeType = m_nodeType; msg.timestamp = now; msg.nonce = m_nonce;
  ByteWriter w; w.U32 (msg.nodeId); w.Str (msg.nodeType); w.Dbl (msg.timestamp); w.U64 (msg.nonce);
  msg.signature = CryptoUtils::Sign (m_keyPair, w.buf);
  msg.cert = m_cert;

  auto bytes = msg.Serialize ();
  Ptr<Packet> p = Create<Packet> (bytes.data (), bytes.size ());
  m_socket->SendTo (p, 0, m_rsuAddress);

  ScheduleRetry ();
}

void
MobileNodeApp::ScheduleRetry ()
{
  // Bounded backoff: retry every 0.5-1.5s (randomized to desynchronize
  // colliding retries) up to a generous attempt cap; each retry bumps the
  // nonce so a stale in-flight request can never be replayed as "fresh".
  if (m_authAttempts >= 20) return;
  Ptr<UniformRandomVariable> backoff = CreateObject<UniformRandomVariable> ();
  backoff->SetAttribute ("Min", DoubleValue (0.5));
  backoff->SetAttribute ("Max", DoubleValue (1.5));
  m_retryEvent = Simulator::Schedule (Seconds (backoff->GetValue ()), [this] () {
    if (!m_authenticated) { m_nonce++; SendAuthRequest (); }
  });
}

void
MobileNodeApp::HandleRead (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> packet;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0) continue;
      std::vector<uint8_t> buf (packet->GetSize ());
      packet->CopyData (buf.data (), buf.size ());
      MsgType type = static_cast<MsgType> (buf[0]);
      ByteReader r (buf); r.U8 ();

      if (type == MsgType::AUTH_ACCEPT)
        {
          AuthAcceptMsg msg = AuthAcceptMsg::Parse (r);
          if (msg.nodeId != m_nodeId || m_authenticated) continue;
          m_authenticated = true;
          m_committedBlockIndex = msg.blockIndex;
          m_sessionToken = DeriveSessionTokenFromHash (m_nodeId, msg.blockHash);
          if (m_retryEvent.IsRunning ()) Simulator::Cancel (m_retryEvent);
          m_dataEvent = Simulator::Schedule (m_dataInterval, &MobileNodeApp::SendDataMsg, this);
        }
      else if (type == MsgType::AUTH_REJECT)
        {
          AuthRejectMsg msg = AuthRejectMsg::Parse (r);
          if (msg.nodeId != m_nodeId || m_authenticated) continue;
          // legitimate node: just retry with a fresh nonce/timestamp
          // (retry already scheduled by SendAuthRequest)
        }
    }
}

void
MobileNodeApp::SendDataMsg ()
{
  if (!m_authenticated) return;
  double now = Simulator::Now ().GetSeconds ();

  DataMsg msg;
  msg.nodeId = m_nodeId; msg.timestamp = now;
  // Simplified telemetry payload; extend with real mobility-model position/
  // speed (GetNode()->GetObject<MobilityModel>()) for richer V2X payloads.
  msg.payload = m_nodeType + "_telemetry_seq";

  ByteWriter w; w.U32 (msg.nodeId); w.Dbl (msg.timestamp); w.Str (msg.payload);
  msg.hmacTag = CryptoUtils::HmacSha256 (m_sessionToken, w.buf);

  auto bytes = msg.Serialize ();
  Ptr<Packet> p = Create<Packet> (bytes.data (), bytes.size ());
  m_socket->SendTo (p, 0, m_rsuAddress);

  m_dataEvent = Simulator::Schedule (m_dataInterval, &MobileNodeApp::SendDataMsg, this);
}
