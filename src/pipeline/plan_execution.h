/* Take an abstract view of a pipeline as a DAG.
 *
 * Note: ordered sets are being used to make unit tests
 * deterministic.
 */
#ifndef PLAN_EXECUTION_H
#define PLAN_EXECUTION_H

#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#include <exception>

using connections_t = std::unordered_map<
  std::string,          // from node
  std::set<std::string> // to nodes
>;

using plan_t = std::vector<
  std::tuple<
    std::string,         // node
    std::set<uint32_t>,  // input buffer numbers
    uint32_t             // output buffer number
  >
>;

class InvalidConnections: public std::runtime_error {
public:
  InvalidConnections(std::string msg);
};

/* Given the connections of audio elements, provide the order
 * in which the pipeline needs to be executed, along with named
 * buffers which should be provided as inputs to each element.
 */
plan_t planExecution(const connections_t& connections);

/* Determine how many buffers need to be allocated.
 */
uint32_t countBuffersInPlan(const plan_t& plan);

#endif