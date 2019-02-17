#include <gtest/gtest.h>
#include "../../src/pipeline/plan_execution.h"

using namespace std;

TEST(PlanExecutionTest, givenEmptyConnections_ItReturnsEmptyPlan) {
  auto connections = connections_t();
  auto plan = planExecution(connections);
  ASSERT_EQ(plan.size(), 0);
}

TEST(PlanExecutionTest, givenSingleElement_ItPlansExecution) {
  connections_t connections = { {"a", {}} };
  auto plan = planExecution(connections);
  plan_t expected = { {"a", {}, 0} };
  ASSERT_EQ(plan, expected);
}

TEST(PlanExecutionTest, givenLinearChainOfElements_ItPlansExecution) {
  auto connections = connections_t();
  connections["a"].insert("b");
  connections["b"].insert("c");
  connections["c"].insert("d");

  auto plan = planExecution(connections);
  plan_t expected = {
    {"a", {}, 0},
    {"b", {0}, 1},
    {"c", {1}, 0},
    {"d", {0}, 1},
  };
  ASSERT_EQ(plan, expected);
}

TEST(PlanExecutionTest, givenComplexChainOfElements_ItplansExecution) {
  auto connections = connections_t();
  connections["a"].insert("b");
  connections["a"].insert("c");
  connections["a"].insert("f");
  connections["f"].insert("d");
  connections["c"].insert("d");
  connections["d"].insert("e");
  connections["b"].insert("e");
  connections["e"].insert("g");

  auto plan = planExecution(connections);
  plan_t expected = {
    {"a", {}, 0},
    {"f", {0}, 1},
    {"c", {0}, 2},
    {"d", {1, 2}, 3},
    {"b", {0}, 1},
    {"e", {1, 3}, 0},
    {"g", {0}, 3},
  };
  ASSERT_EQ(plan, expected);
}

TEST(PlanExecutionTest, givenConnectionsWithALoop_ItThrowsException) {
  auto connections = connections_t();
  connections["a"].insert("b");
  connections["b"].insert("a");
  connections["c"].insert("a");

  ASSERT_THROW(
    planExecution(connections),
    InvalidConnections
  );
}

TEST(PlanExecutionTest, givenAPlan_ItCountsNumberOfBuffersRequired) {
  auto connections = connections_t();
  connections["a"].insert("c");
  connections["b"].insert("c");
  connections["c"].insert("d");
  connections["d"].insert("e");
  auto plan = planExecution(connections);

  ASSERT_EQ(countBuffersInPlan(plan), 3);
};

TEST(PlanExecutionTest, givenDisconnectedChainsOfElements_ItFindsTheTerminalNodes) {
  connections_t connections = {
    // chain
    { "comp1_a", { "comp1_b" }},
    // tree
    { "comp2_a", { "comp2_b" }},
    { "comp2_b", { "comp2_d" }},
    { "comp2_c", { "comp2_b", "comp2_d" }},
    // single node
    { "comp3_a", {}},
    // cycle
    { "comp4_a", { "comp4_b" }},
    { "comp4_b", { "comp4_a" }},
  };
  set<string> expected = {
    "comp1_b",
    "comp2_d",
    "comp3_a",
  };

  ASSERT_EQ(findTerminalNodes(connections), expected);
}
