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
    <React.Fragment>
      <div className="flex-container-row">
        <ControlGroup label="Gen 1">
          <SelectWithLabel label="Wave" {...selectArgs(PARAM_GEN1_WAVE_TYPE, PARAM_WAVE_OPTIONS)} />
          <KnobWithLabel label="Offset" {...knobArgs(PARAM_GEN1_COARSE_OFFSET)} />
          <KnobWithLabel label="Tune" {...knobArgs(PARAM_GEN1_FINE_OFFSET)} />
          <KnobWithLabel label="Amp" {...knobArgs(PARAM_GEN1_AMP)} />
        </ControlGroup>
        <ControlGroup label="Gen 2">
          <SelectWithLabel label="Wave" {...selectArgs(PARAM_GEN2_WAVE_TYPE, PARAM_WAVE_OPTIONS)} />
          <KnobWithLabel label="Offset" {...knobArgs(PARAM_GEN2_COARSE_OFFSET)} />
          <KnobWithLabel label="Tune" {...knobArgs(PARAM_GEN2_FINE_OFFSET)} />
          <KnobWithLabel label="Amp" {...knobArgs(PARAM_GEN2_AMP)} />
        </ControlGroup>
      </div>
      <div className="flex-container-row">
        <ControlGroup label="Gen 3">
          <SelectWithLabel label="Wave" {...selectArgs(PARAM_GEN3_WAVE_TYPE, PARAM_WAVE_OPTIONS)} />
          <KnobWithLabel label="Offset" {...knobArgs(PARAM_GEN3_COARSE_OFFSET)} />
          <KnobWithLabel label="Tune" {...knobArgs(PARAM_GEN3_FINE_OFFSET)} />
          <KnobWithLabel label="Amp" {...knobArgs(PARAM_GEN3_AMP)} />
        </ControlGroup>
        <ControlGroup label="Filter">
          <KnobWithLabel label="Cutoff" {...knobArgs(PARAM_FILTER_CUTOFF)} />
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
        <ControlGroup label="FM LFO">
          <SelectWithLabel label="Wave" {...selectArgs(PARAM_LFO_WAVE_TYPE, PARAM_WAVE_OPTIONS)} />
          <KnobWithLabel label="Amp" {...knobArgs(PARAM_LFO_AMP)} />
          <KnobWithLabel label="Freq" {...knobArgs(PARAM_LFO_FREQ)} />
        </ControlGroup>
        <ControlGroup label="Master">
          <KnobWithLabel label="Dist" {...knobArgs(PARAM_DISTORTION)} />
          <KnobWithLabel label="Amp" {...knobArgs(PARAM_MASTER_AMP)} />
        </ControlGroup>
      </div>
    </React.Fragment>
  );
}
