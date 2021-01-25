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
  connections["a"].emplace_back("b", 0);
  connections["b"].emplace_back("c", 0);
  connections["c"].emplace_back("d", 0);

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
  connections["a"].emplace_back("b", 0);
  connections["a"].emplace_back("c", 0);
  connections["a"].emplace_back("f", 0);
  connections["f"].emplace_back("d", 0);
  connections["c"].emplace_back("d", 1);
  connections["d"].emplace_back("e", 1);
  connections["b"].emplace_back("e", 0);
  connections["e"].emplace_back("g", 0);

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

TEST(PlanExecutionTest, givenSingleElementWithALoop_ItThrowsException) {
  auto connections = connections_t();
  connections["a"].emplace_back("a", 0);

  ASSERT_THROW(
    planExecution(connections),
    InvalidConnections
  );
}

TEST(PlanExecutionTest, giveMultipleElementsWithALoop_ItThrowsException) {
  auto connections = connections_t();
  connections["a"].emplace_back("b", 0);
  connections["b"].emplace_back("a", 0);
  connections["c"].emplace_back("a", 1);

  ASSERT_THROW(
    planExecution(connections),
    InvalidConnections
  );
}

TEST(PlanExecutionTest, givenConnectionsWithConflictingPorts_ItThrowsException) {
  auto connections = connections_t();
  connections["a"].emplace_back("c", 0);
  connections["b"].emplace_back("c", 0);

  ASSERT_THROW(
    planExecution(connections),
    InvalidConnections
  );
}

TEST(PlanExecutionTest, givenAPlan_ItCountsNumberOfBuffersRequired) {
  auto connections = connections_t();
  connections["a"].emplace_back("c", 0);
  connections["b"].emplace_back("c", 1);
  connections["c"].emplace_back("d", 0);
  connections["d"].emplace_back("e", 0);
  auto plan = planExecution(connections);

  ASSERT_EQ(countBuffersInPlan(plan), 3);
};

TEST(PlanExecutionTest, givenDisconnectedChainsOfElements_ItFindsTheTerminalNodes) {
  connections_t connections = {
    // chain
    {"comp1_a", {{"comp1_b", 0}}},
    // tree
    {"comp2_a", {{"comp2_b", 0}}},
    {"comp2_b", {{"comp2_d", 0}}},
    {"comp2_c", {{"comp2_b", 0}, {"comp2_d", 0}}},
    // single node
    {"comp3_a", {}},
    // cycle
    {"comp4_a", {{"comp4_b", 0}}},
    {"comp4_b", {{"comp4_a", 0}}},
  };
  set<string> expected = {
    "comp1_b",
    "comp2_d",
    "comp3_a",
  };

  ASSERT_EQ(findTerminalNodes(connections), expected);
}
