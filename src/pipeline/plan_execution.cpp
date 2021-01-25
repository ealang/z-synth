#include "./plan_execution.h"

#include <sstream>
#include <stack>

using namespace std;

using dependency_map_t = unordered_map<string, set<string>>;
using nodeport_dependency_map_t = unordered_map<string, unordered_map<string, set<uint32_t>>>;

class BufferPool {
  uint32_t next = 0;
  stack<uint32_t> pool;
public:
  uint32_t take() {
    if (pool.size() == 0) {
      return next++;
    }
    auto num = pool.top();
    pool.pop();
    return num;
  }

  void put(uint32_t num) {
    pool.push(num);
  }
};

InvalidConnections::InvalidConnections(string msg): runtime_error(msg) {
}

set<string> allNodes(const connections_t& connections) {
  set<string> all;
  for (const auto& outgoingConnections: connections) {
    all.insert(outgoingConnections.first);
    for (const auto& connection: outgoingConnections.second) {
      all.insert(get<0>(connection));
    }
  }
  return all;
}

static set<string> dependencyFreeNodes(
  const connections_t& connections,
  dependency_map_t& dependencies
) {
  set<string> nodes;
  for (const auto& outgoingConnections: connections) {
    auto node = outgoingConnections.first;
    if (dependencies[node].size() == 0) {
      nodes.insert(node);
    }
  }
  return nodes;
}

// Lookup node -> dependencies of node
static dependency_map_t buildNodeDependencyLookup(const connections_t& connections) {
  dependency_map_t dependencies;
  for (const auto& outgoingConnections: connections) {
    auto node = outgoingConnections.first;
    for (const auto& connection: outgoingConnections.second) {
      dependencies[connection.first].insert(node);
    }
  }
  return dependencies;
}

// Lookup node -> dependencies of node -> input port numbers
static nodeport_dependency_map_t buildNodePortDependencyLookup(const connections_t& connections) {
  nodeport_dependency_map_t portLookup;
  for (const auto& outgoingConnections: connections) {
    const auto fromNode = outgoingConnections.first;
    for (const auto& connection: outgoingConnections.second) {
      const auto toNode = connection.first;
      const auto portNumber = connection.second;
      portLookup[toNode][fromNode].emplace(portNumber);
    }
  }
  return portLookup;
}

bool canExecuteNode(const string& node, const set<string>& executed, dependency_map_t& dependencies) {
  for (auto dependency: dependencies[node]) {
    if (executed.count(dependency) == 0) {
      return false;
    }
  }
  return true;
}

// Throw if multiple connections are assigned to the same inbound port.
static void throwIfHasConflictingPorts(const connections_t& connections) {
  unordered_map<string, set<uint32_t>> consumedPorts;
  for (const auto& outgoingConnections: connections) {
    for (const auto& connection: outgoingConnections.second) {
      const auto toNode = connection.first;
      const auto portNumber = connection.second;
      if (consumedPorts[toNode].count(portNumber) > 0) {
        ostringstream s;
        s << "Multiple connections to node " << toNode << " port " << portNumber;
        throw InvalidConnections(s.str());
      }
      consumedPorts[toNode].emplace(portNumber);
    }
  }
}

static vector<uint32_t> inputBuffersForNode(
  const string& node,
  nodeport_dependency_map_t& portNumbers,
  unordered_map<string, uint32_t>& outputBuffers
) {
  vector<uint32_t> buffers;

  if (portNumbers[node].size() == 0) {
    return buffers;
  }

  // Find the max port number from any incoming connections
  uint32_t maxPortNumber = 0;
  for (const auto& incommingConection: portNumbers[node]) {
    for (const auto portNumber: incommingConection.second) {
      maxPortNumber = max(maxPortNumber, portNumber);
    }
  }

  // Assign buffers
  buffers.resize(maxPortNumber + 1, NULL_BUFFER_NUM);
  for (const auto& incommingConection: portNumbers[node]) {
    const auto& fromNode = incommingConection.first;
    for (const auto portNumber: incommingConection.second) {
      buffers[portNumber] = outputBuffers[fromNode];
    }
  }

  return buffers;
}

static bool anotherNodeIsWaitingForThisNode(
  const string& node,
  dependency_map_t& dependencies,
  const set<string>& waitingNodes)
{
  for (auto waitingNode: waitingNodes) {
    if (dependencies[waitingNode].count(node) == 1) {
      return true;
    }
  }
  return false;
}

plan_t planExecution(const connections_t& connections) {
  throwIfHasConflictingPorts(connections);

  plan_t plan;
  BufferPool bufferPool;
  unordered_map<string, uint32_t> outputBuffers;
  set<string> executed, waiting = allNodes(connections);
  auto dependencies = buildNodeDependencyLookup(connections);
  auto portNumbers = buildNodePortDependencyLookup(connections);

  stack<string> toVisit;
  {
    for (auto node: dependencyFreeNodes(connections, dependencies)) {
      toVisit.push(node);
    }
  }

  while (toVisit.size() > 0) {
    auto node = toVisit.top();
    toVisit.pop();

    if (canExecuteNode(node, executed, dependencies)) {
      waiting.erase(node);
      executed.insert(node);

      // visit downstream nodes
      auto runAfter = connections.find(node);
      if (runAfter != connections.end()) {
        for (const auto& otherNode: runAfter->second) {
          toVisit.push(otherNode.first);
        }
      }

      // track buffers
      auto inputBuffers = inputBuffersForNode(node, portNumbers, outputBuffers);
      auto outputBuffer = bufferPool.take();
      plan.emplace_back(node, inputBuffers, outputBuffer);
      outputBuffers[node] = outputBuffer;
      for (const auto& dependency: dependencies[node]) {
        if (!anotherNodeIsWaitingForThisNode(dependency, dependencies, waiting)) {
          bufferPool.put(outputBuffers[dependency]);
        }
      }
    }
  }

  if (waiting.size() > 0) {
    throw InvalidConnections("Pipeline contains a loop");
  }

  return plan;
}

uint32_t countBuffersInPlan(const plan_t& plan) {
  set<uint32_t> buffers;
  for (auto step: plan) {
    auto inputs = get<1>(step);
    auto output = get<2>(step);
    for (auto input: inputs) {
      buffers.insert(input);
    }
    buffers.insert(output);
  }
  return buffers.size();
}

set<string> findTerminalNodes(const connections_t& connections) {
  set<string> terminals;
  for (const auto &node: allNodes(connections)) {
    auto outs = connections.find(node);
    if (outs == connections.end() || outs->second.size() == 0) {
      terminals.insert(node);
    }
  }
  return terminals;
}
