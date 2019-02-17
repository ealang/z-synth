/**
 * Blackbox tests for pipeline & builder.
 */
#include <exception>
#include <memory>
#include <gtest/gtest.h>
#include "../../src/pipeline/pipeline_element.h"
#include "../../src/pipeline/pipeline_builder.h"

using namespace std;

/* Present a constant value at the output. */
class ConstElem: public MidiAudioElement<int> {
  int val;
public:
  ConstElem(int val): val(val) {}
  uint32_t maxInputs() override { return 0; };
  void generate(uint32_t numSamples, int* out, uint32_t, inputs_t<int>) override {
    for (uint32_t j = 0; j < numSamples; j++) {
      out[j] = val;
    }
  }
};

/* Sum all inputs plus a given value. */
class SumElem: public MidiAudioElement<int> {
  int add;
public:
  SumElem(int add): add(add) {}
  uint32_t maxInputs() override { return -1; };
  void generate(uint32_t numSamples, int* out, uint32_t numInputs, inputs_t<int> inputs) override {
    for (uint32_t j = 0; j < numSamples; j++) {
      out[j] = add;
      for (uint32_t i = 0; i < numInputs; i++) {
        out[j] += inputs[i][j];
      }
    }
  }
};

/* Run the pipeline on a single sample. */
int runPipeline(PipelineBuilder<int>& builder) {
  uint32_t numSamples = 1;
  uint32_t channelCount = 1;
  shared_ptr<AudioElement<int>> pipeline = builder.build(numSamples, channelCount);

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
  builder.registerElem("input", static_cast<shared_ptr<AudioElement<int>>>(make_shared<ConstElem>(42)));
  builder.setOutputElem("input");

  ASSERT_EQ(runPipeline(builder), 42);
}

TEST(PipelineBuilderTest, givenALinearChain_ItBuildsAPipeline) {
  auto builder = PipelineBuilder<int>();
  builder.registerElem("input", static_cast<shared_ptr<AudioElement<int>>>(make_shared<ConstElem>(42)));
  builder.registerElem("add", static_cast<shared_ptr<AudioElement<int>>>(make_shared<SumElem>(1)));
  builder.connectElems("input", "add");
  builder.setOutputElem("add");

  ASSERT_EQ(runPipeline(builder), 43);
}

TEST(PipelineBuilderTest, givenAFanInGraph_ItBuildsAPipeline) {
  auto builder = PipelineBuilder<int>();
  builder.registerElem(
    "input1",
    static_cast<shared_ptr<AudioElement<int>>>(make_shared<ConstElem>(10))
  );
  builder.registerElem(
    "input2",
    static_cast<shared_ptr<AudioElement<int>>>(make_shared<ConstElem>(20))
  );
  builder.registerElem(
    "add1",
    static_cast<shared_ptr<AudioElement<int>>>(make_shared<SumElem>(1))
  );
  builder.registerElem(
    "add2",
    static_cast<shared_ptr<AudioElement<int>>>(make_shared<SumElem>(40))
  );
  builder.connectElems("input1", "add1");
  builder.connectElems("input2", "add2");
  builder.connectElems("add1", "add2");
  builder.setOutputElem("add2");

  ASSERT_EQ(runPipeline(builder), 71);
}
