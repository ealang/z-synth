function ParamControls({initParams, onParamChanged}) {
  const mix = React.useState(initParams[PARAM_GEN_MIX]);
  const cutoff = React.useState(initParams[PARAM_FILTER_CUTOFF]);
  const wave1Sel = React.useState(initParams[PARAM_GEN1_WAVE_TYPE]);

  const knobs = {
    [PARAM_GEN_MIX]: mix,
    [PARAM_FILTER_CUTOFF]: cutoff,
    [PARAM_GEN1_WAVE_TYPE]: wave1Sel,
  };

  const onKnobChange = (param) => {
    return (newValue) => {
      knobs[param][1](newValue);
      onParamChanged(param, newValue);
    };
  };

  const onSelectChange = (param) => {
    return (event) => {
      const newValue = event.target.value;
      knobs[param][1](newValue);
      onParamChanged(param, newValue);
    };
  }

  return (
    <div>
      <Knob value={mix[0]} onValueChanged={onKnobChange(PARAM_GEN_MIX)} />
      <Knob value={cutoff[0]} onValueChanged={onKnobChange(PARAM_FILTER_CUTOFF)} />
      <Select value={wave1Sel[0]} values={PARAM_WAVE_OPTIONS} onValueChanged={onSelectChange(PARAM_GEN1_WAVE_TYPE)} />
    </div>
  );
}
