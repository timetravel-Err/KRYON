#include "metrics.h"
#include <iostream>

MetricsCollector &
MetricsCollector::Get ()
{
  static MetricsCollector instance;
  return instance;
}

void
MetricsCollector::LogAuthAttempt (double time, uint32_t nodeId, const std::string &nodeType)
{
  std::lock_guard<std::mutex> lk (m_mutex);
  (void) time; (void) nodeId; (void) nodeType; // reserved for future per-attempt tracing
}

void
MetricsCollector::LogAuthResult (double time, uint32_t nodeId, bool success,
                                  double latencySec, const std::string &reason)
{
  std::lock_guard<std::mutex> lk (m_mutex);
  m_authEvents.push_back ({time, nodeId, "", success, latencySec, reason});
}

void
MetricsCollector::LogConsensusRound (double time, uint64_t view, uint64_t seq,
                                      uint32_t primaryId, double latencySec, size_t numCommitVotes,
                                      uint32_t reportingRsuId)
{
  std::lock_guard<std::mutex> lk (m_mutex);
  m_consensusEvents.push_back ({time, view, seq, primaryId, latencySec, numCommitVotes, reportingRsuId});
}

void
MetricsCollector::LogBlockCommitted (double time, uint64_t blockIndex, size_t numTx, uint32_t reportingRsuId)
{
  std::lock_guard<std::mutex> lk (m_mutex);
  m_blockEvents.push_back ({time, blockIndex, numTx, reportingRsuId});
}

void
MetricsCollector::LogDataMsg (double time, uint32_t nodeId, bool hmacValid)
{
  std::lock_guard<std::mutex> lk (m_mutex);
  m_dataEvents.push_back ({time, nodeId, hmacValid});
}

void
MetricsCollector::LogAttackEvent (double time, const std::string &attackType,
                                   uint32_t sourceNodeId, const std::string &outcome)
{
  std::lock_guard<std::mutex> lk (m_mutex);
  m_attackEvents.push_back ({time, attackType, sourceNodeId, outcome});
}

void
MetricsCollector::Flush ()
{
  std::lock_guard<std::mutex> lk (m_mutex);

  {
    std::ofstream f (m_outDir + "/auth_events.csv");
    f << "time,nodeId,success,latencySec,reason\n";
    for (auto &e : m_authEvents)
      f << e.time << "," << e.nodeId << "," << (e.success ? 1 : 0) << ","
        << e.latencySec << "," << e.reason << "\n";
  }
  {
    std::ofstream f (m_outDir + "/consensus_events.csv");
    f << "time,view,seq,primaryId,latencySec,numCommitVotes,reportingRsuId\n";
    for (auto &e : m_consensusEvents)
      f << e.time << "," << e.view << "," << e.seq << "," << e.primaryId << ","
        << e.latencySec << "," << e.numCommitVotes << "," << e.reportingRsuId << "\n";
  }
  {
    std::ofstream f (m_outDir + "/block_events.csv");
    f << "time,blockIndex,numTx,reportingRsuId\n";
    for (auto &e : m_blockEvents)
      f << e.time << "," << e.blockIndex << "," << e.numTx << "," << e.reportingRsuId << "\n";
  }
  {
    std::ofstream f (m_outDir + "/data_events.csv");
    f << "time,nodeId,hmacValid\n";
    for (auto &e : m_dataEvents)
      f << e.time << "," << e.nodeId << "," << (e.hmacValid ? 1 : 0) << "\n";
  }
  {
    std::ofstream f (m_outDir + "/attack_events.csv");
    f << "time,attackType,sourceNodeId,outcome\n";
    for (auto &e : m_attackEvents)
      f << e.time << "," << e.attackType << "," << e.sourceNodeId << "," << e.outcome << "\n";
  }

  std::cout << "[MetricsCollector] Flushed "
            << m_authEvents.size () << " auth, "
            << m_consensusEvents.size () << " consensus, "
            << m_blockEvents.size () << " block, "
            << m_dataEvents.size () << " data, "
            << m_attackEvents.size () << " attack events to " << m_outDir << std::endl;
}
