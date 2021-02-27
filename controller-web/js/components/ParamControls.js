function ParamControls({params, onParamChanged}) {
  const selectArgs = (param, values) => {
    return {
      "value": params[param],
      "values": values,
      "onValueChanged": (event) => {
        const value = event.target.value;
        onParamChanged(param, value);
      },
    };
  };

  const knobArgs = (param) => {
    return {
      "value": params[param],
      "onValueChanged": (value) => {
        onParamChanged(param, value);
      },
    };
  };

  return (
    <div>
      <div className="flex-container-row">
        <ControlGroup label="Gen 1">
          <SelectWithLabel label="Wave 1" {...selectArgs(PARAM_GEN1_WAVE_TYPE, PARAM_WAVE_OPTIONS)} />
          <KnobWithLabel label="Offset 1" {...knobArgs(PARAM_GEN1_COARSE_OFFSET)} />
          <KnobWithLabel label="Tune 1" {...knobArgs(PARAM_GEN1_FINE_OFFSET)} />
        </ControlGroup>
        <ControlGroup label="Gen 2">
          <SelectWithLabel label="Wave 2" {...selectArgs(PARAM_GEN2_WAVE_TYPE, PARAM_WAVE_OPTIONS)} />
          <KnobWithLabel label="Offset 2" {...knobArgs(PARAM_GEN2_COARSE_OFFSET)} />
          <KnobWithLabel label="Tune 2" {...knobArgs(PARAM_GEN2_FINE_OFFSET)} />
        </ControlGroup>
        <ControlGroup label="Generators">
          <KnobWithLabel label="Mix" {...knobArgs(PARAM_GEN_MIX)} />
          <KnobWithLabel label="Filter" {...knobArgs(PARAM_FILTER_CUTOFF)} />
        </ControlGroup>
      </div>
      <div className="flex-container-row">
        <ControlGroup label="Amp Env">
          <KnobWithLabel label="Attack" {...knobArgs(PARAM_AMP_ENV_ATTACK)} />
          <KnobWithLabel label="Decay" {...knobArgs(PARAM_AMP_ENV_DECAY)} />
          <KnobWithLabel label="Sustain" {...knobArgs(PARAM_AMP_ENV_SUSTAIN)} />
          <KnobWithLabel label="Release" {...knobArgs(PARAM_AMP_ENV_RELEASE)} />
        </ControlGroup>
        <ControlGroup label="Filter Env">
          <KnobWithLabel label="Attack" {...knobArgs(PARAM_FILTER_ENV_ATTACK)} />
          <KnobWithLabel label="Decay" {...knobArgs(PARAM_FILTER_ENV_DECAY)} />
          <KnobWithLabel label="Sustain" {...knobArgs(PARAM_FILTER_ENV_SUSTAIN)} />
          <KnobWithLabel label="Release" {...knobArgs(PARAM_FILTER_ENV_RELEASE)} />
        </ControlGroup>
      </div>
      <div className="flex-container-row">
        <ControlGroup label="LFO">
          <SelectWithLabel label="Wave" {...selectArgs(PARAM_LFO_WAVE_TYPE, PARAM_WAVE_OPTIONS)} />
          <KnobWithLabel label="Amp" {...knobArgs(PARAM_LFO_AMP)} />
          <KnobWithLabel label="Freq" {...knobArgs(PARAM_LFO_FREQ)} />
        </ControlGroup>
        <ControlGroup label="Master">
          <KnobWithLabel label="Dist" {...knobArgs(PARAM_DISTORTION)} />
          <KnobWithLabel label="Amp" {...knobArgs(PARAM_MASTER_AMP)} />
        </ControlGroup>
      </div>
    </div>
  );
}
