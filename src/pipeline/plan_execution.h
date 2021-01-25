/* Take an abstract view of a pipeline as a DAG. */
#ifndef PLAN_EXECUTION_H
#define PLAN_EXECUTION_H

#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#include <exception>

const uint32_t NULL_BUFFER_NUM = -1;

// Store a set of directed connections between nodes
using connections_t = std::unordered_map<
  std::string,                                  // from node name
  std::vector<std::pair<std::string, uint32_t>> // to node name, port number
>;

// Instructions for executing a pipeline
using plan_t = std::vector<
  std::tuple<
    std::string,            // node
    std::vector<uint32_t>,  // input buffer numbers
    uint32_t                // output buffer number
  >
>;

class InvalidConnections: public std::runtime_error {
public:
  InvalidConnections(std::string msg);
};

/* Return all nodes present in the connections. */
std::set<std::string> allNodes(const connections_t& connections);

/* Given the connections of audio elements, provide the order
 * in which the pipeline needs to be executed, along with named
 * buffers which should be provided as inputs to each element.
 * NULL_BUFFER_NUM indicates a disconnected input.
 */
plan_t planExecution(const connections_t& connections);

/* Determine how many buffers need to be allocated.
 */
uint32_t countBuffersInPlan(const plan_t& plan);

/* Identify nodes have no connections to other nodes.
 */
std::set<std::string> findTerminalNodes(const connections_t& connections);

#endif
