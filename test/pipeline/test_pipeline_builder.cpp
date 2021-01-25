/**
 * Integration tests for pipeline & builder.
 */
#include <exception>
#include <memory>
#include <gtest/gtest.h>
#include "../../src/pipeline/pipeline_element.h"
#include "../../src/pipeline/pipeline_builder.h"

using namespace std;

/* Present a constant value at the output. */
class ConstElem: public AudioElement<int> {
  int val;
public:
  ConstElem(int val): val(val) {}
  uint32_t maxInputs() const override { return 0; };
  void generate(uint32_t numSamples, int* out, uint32_t, inputs_t<int>) override {
    for (uint32_t j = 0; j < numSamples; j++) {
      out[j] = val;
    }
  }
};

/* Sum all inputs plus a given value. */
class SumElem: public AudioElement<int> {
  int add;
public:
  SumElem(int add): add(add) {}
  uint32_t maxInputs() const override { return -1; };
  void generate(uint32_t numSamples, int* out, uint32_t numInputs, inputs_t<int> inputs) override {
    for (uint32_t j = 0; j < numSamples; j++) {
      out[j] = add;
      for (uint32_t i = 0; i < numInputs; i++) {
        out[j] += inputs[i][j];
      }
    }
  }
};

/* Sum values giving each port a different weight. */
class SumWeightedPortsElem: public AudioElement<int> {
public:
  uint32_t maxInputs() const override { return -1; };
  void generate(uint32_t numSamples, int* out, uint32_t numInputs, inputs_t<int> inputs) override {
    for (uint32_t i = 0; i < numSamples; i++) {
      int v = 0;
      int multiplier = 1;
      for (uint32_t j = 0; j < numInputs; j++) {
        if (inputs[j] != nullptr) {
          v += inputs[j][i] * multiplier;
        }
        multiplier *= 10;
      }
      out[i] = v;
    }
  }
};

/* Run the pipeline on a single sample. */
int runPipeline(PipelineBuilder<int>& builder) {
  uint32_t numSamples = 1;
  shared_ptr<AudioElement<int>> pipeline = builder.build(numSamples);

  uint32_t numInputs = 1;
  inputs_t<int> inBuffers = nullptr;
  int outBuffer[1] = {0};
  pipeline->generate(numSamples, outBuffer, numInputs, inBuffers);
  return outBuffer[0];
}

TEST(PipelineBuilderTest, givenNoElements_ItThrowsAnException) {
  auto builder = PipelineBuilder<int>();
  ASSERT_THROW(
    runPipeline(builder),
    runtime_error
  );
}

TEST(PipelineBuilderTest, givenASingleElement_ItBuildsAPipeline) {
  auto builder = PipelineBuilder<int>();
  builder.registerElem("input", make_shared<ConstElem>(42));
  builder.setOutputElem("input");

  ASSERT_EQ(runPipeline(builder), 42);
}

TEST(PipelineBuilderTest, givenALinearChain_ItBuildsAPipeline) {
  auto builder = PipelineBuilder<int>();
  builder.registerElem("input", make_shared<ConstElem>(42));
  builder.registerElem("add", make_shared<SumElem>(1));
  builder.connectElems("input", "add", 0);
  builder.setOutputElem("add");

  ASSERT_EQ(runPipeline(builder), 43);
}

TEST(PipelineBuilderTest, givenAFanInGraph_ItBuildsAPipeline) {
  auto builder = PipelineBuilder<int>();
  builder.registerElem(
    "input1",
    make_shared<ConstElem>(10)
  );
  builder.registerElem(
    "input2",
    make_shared<ConstElem>(20)
  );
  builder.registerElem(
    "add1",
    make_shared<SumElem>(1)
  );
  builder.registerElem(
    "add2",
    make_shared<SumElem>(40)
  );
  builder.connectElems("input1", "add1", 0);
  builder.connectElems("input2", "add2", 0);
  builder.connectElems("add1", "add2", 1);
  builder.setOutputElem("add2");

  ASSERT_EQ(runPipeline(builder), 71);
}

TEST(PipelineBuilderTest, givenElementWithManyPorts_ItAssignsAsExpected) {
  auto builder = PipelineBuilder<int>();
  builder.registerElem(
    "const1",
    make_shared<ConstElem>(1)
  );
  builder.registerElem(
    "const2",
    make_shared<ConstElem>(2)
  );
  builder.registerElem(
    "adder",
    make_shared<SumWeightedPortsElem>()
  );
  builder.connectElems("const1", "adder", 0);
  builder.connectElems("const1", "adder", 3);
  builder.connectElems("const2", "adder", 4);
  builder.connectElems("const2", "adder", 6);
  builder.setOutputElem("adder");

  ASSERT_EQ(runPipeline(builder), 2021001);
}

TEST(PipelineBuilderTest, givenMoreConnectionsThanInputs_ItThrowsAnException) {
  auto builder = PipelineBuilder<int>();
  builder.registerElem("const1", make_shared<ConstElem>(1));
  builder.registerElem("const2", make_shared<ConstElem>(2));
  builder.connectElems("const1", "const2", 0);
  builder.setOutputElem("const2");
  bool thrown = false;
  try {
    runPipeline(builder);
  } catch (const runtime_error& e) {
    string msg = e.what();
    ASSERT_EQ(msg, "Element \"const2\" received 1 input(s) but supports a max of 0");
    thrown = true;
  }
  ASSERT_TRUE(thrown);
}
