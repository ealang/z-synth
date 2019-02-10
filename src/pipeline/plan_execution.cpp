#include <stack>

#include "./plan_execution.h"

using namespace std;

using dependency_map_t = unordered_map<string, set<string>>;

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

InvalidConnections::InvalidConnections(std::string msg): runtime_error(msg) {
}

set<string> allNodes(const connections_t& connections) {
  set<string> all;
  for (auto connection: connections) {
    all.insert(connection.first);
    for (auto after: connection.second) {
      all.insert(after);
    }
  }
  return all;
}

set<string> dependencyFreeNodes(
  const connections_t& connections,
  dependency_map_t& dependencies
) {
  set<string> nodes;
  for (auto connection: connections) {
    auto node = connection.first;
    if (dependencies[node].size() == 0) {
      nodes.insert(node);
    }
  }
  return nodes;
}

dependency_map_t nodeDependencies(const connections_t& connections) {
  dependency_map_t dependencies;
  for (auto connection: connections) {
    auto node = connection.first;
    for (auto runAfter: connection.second) {
      dependencies[runAfter].insert(node);
    }
  }
  return dependencies;
}

bool canExecuteNode(string& node, set<string>& executed, dependency_map_t& dependencies) {
  for (auto dependency: dependencies[node]) {
    if (executed.count(dependency) == 0) {
      return false;
    }
  }
  return true;
}

set<uint32_t> inputBuffersForNode(
  string& node,
  dependency_map_t& dependencies,
  unordered_map<string, uint32_t>& outputBuffers
) {
  set<uint32_t> buffers;
  for (auto inputNode: dependencies[node]) {
    buffers.insert(outputBuffers[inputNode]);
  }
  return buffers;
}

bool anotherNodeIsWaitingForThisNode(
  string& node,
  dependency_map_t& dependencies,
  set<string>& waitingNodes)
{
  for (auto waitingNode: waitingNodes) {
    if (dependencies[waitingNode].count(node) == 1) {
      return true;
    }
  }
  return false;
}

plan_t planExecution(const connections_t& connections) {
  plan_t plan;
  BufferPool bufferPool;
  unordered_map<string, uint32_t> outputBuffers;
  set<string> executed, waiting = allNodes(connections);
  dependency_map_t dependencies = nodeDependencies(connections);

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
        for (auto otherNode: runAfter->second) {
          toVisit.push(otherNode);
        }
      }

      // track buffers
      auto inputBuffers = inputBuffersForNode(node, dependencies, outputBuffers);
      auto outputBuffer = bufferPool.take();
      plan.push_back(make_tuple(
        node, inputBuffers, outputBuffer
      ));
      outputBuffers[node] = outputBuffer;
      for (auto dependency: dependencies[node]) {
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
