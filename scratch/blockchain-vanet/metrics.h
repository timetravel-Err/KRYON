#ifndef METRICS_H
#define METRICS_H

#include <string>
#include <vector>
#include <fstream>
#include <mutex>

// ---------------------------------------------------------------------------
// MetricsCollector: process-wide singleton (simulation is single-threaded,
// so no real locking needed, mutex kept for safety/clarity) that logs
// timestamped events during the run and dumps them to CSV files at the end.
// Complements ns-3 FlowMonitor, which handles network-layer PDR/throughput.
// ---------------------------------------------------------------------------
class MetricsCollector
{
public:
  static MetricsCollector &Get ();

  void SetOutputDir (const std::string &dir) { m_outDir = dir; }

  // Authentication pipeline
  void LogAuthAttempt (double time, uint32_t nodeId, const std::string &nodeType);
  void LogAuthResult (double time, uint32_t nodeId, bool success,
                       double latencySec, const std::string &reason);

  // PBFT consensus
  void LogConsensusRound (double time, uint64_t view, uint64_t seq,
                           uint32_t primaryId, double latencySec, size_t numCommitVotes);

  // Blockchain growth
  void LogBlockCommitted (double time, uint64_t blockIndex, size_t numTx);

  // Data-plane message (post-auth telemetry)
  void LogDataMsg (double time, uint32_t nodeId, bool hmacValid);

  // Attacks
  void LogAttackEvent (double time, const std::string &attackType, uint32_t sourceNodeId,
                        const std::string &outcome /* "detected" | "succeeded" */);

  // Flush all buffered events to CSV files under m_outDir. Call once at
  // simulation end (e.g. from a Simulator::Schedule'd stop hook).
  void Flush ();

private:
  MetricsCollector () = default;
  std::string m_outDir = ".";

  struct AuthEvent { double time; uint32_t nodeId; std::string nodeType; bool success;
                      double latencySec; std::string reason; };
  struct ConsensusEvent { double time; uint64_t view; uint64_t seq; uint32_t primaryId;
                           double latencySec; size_t numCommitVotes; };
  struct BlockEvent { double time; uint64_t blockIndex; size_t numTx; };
  struct DataEvent { double time; uint32_t nodeId; bool hmacValid; };
  struct AttackEvent { double time; std::string attackType; uint32_t sourceNodeId; std::string outcome; };

  std::vector<AuthEvent> m_authEvents;
  std::vector<ConsensusEvent> m_consensusEvents;
  std::vector<BlockEvent> m_blockEvents;
  std::vector<DataEvent> m_dataEvents;
  std::vector<AttackEvent> m_attackEvents;
  std::mutex m_mutex;
};

#endif // METRICS_H
