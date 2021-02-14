function ParamControls({initParams, onParamChanged}) {
  const wave1Sel = React.useState(initParams[PARAM_GEN1_WAVE_TYPE]);
  const wave2Sel = React.useState(initParams[PARAM_GEN2_WAVE_TYPE]);

  const mix = React.useState(initParams[PARAM_GEN_MIX]);
  const cutoff = React.useState(initParams[PARAM_FILTER_CUTOFF]);

  const gen1FineOffset = React.useState(initParams[PARAM_GEN1_FINE_OFFSET]);
  const gen1CoarseOffset = React.useState(initParams[PARAM_GEN1_COARSE_OFFSET]);
  const gen2FineOffset = React.useState(initParams[PARAM_GEN2_FINE_OFFSET]);
  const gen2CoarseOffset = React.useState(initParams[PARAM_GEN2_COARSE_OFFSET]);

  const ampAttack = React.useState(initParams[PARAM_AMP_ENV_ATTACK]);
  const ampDecay = React.useState(initParams[PARAM_AMP_ENV_DECAY]);
  const ampSustain = React.useState(initParams[PARAM_AMP_ENV_SUSTAIN]);
  const ampRelease = React.useState(initParams[PARAM_AMP_ENV_RELEASE]);

  const filterAttack = React.useState(initParams[PARAM_FILTER_ENV_ATTACK]);
  const filterDecay = React.useState(initParams[PARAM_FILTER_ENV_DECAY]);
  const filterSustain = React.useState(initParams[PARAM_FILTER_ENV_SUSTAIN]);
  const filterRelease = React.useState(initParams[PARAM_FILTER_ENV_RELEASE]);

  const distortion = React.useState(initParams[PARAM_DISTORTION]);

  const lfoAmp = React.useState(initParams[PARAM_LFO_AMP]);
  const lfoFreq = React.useState(initParams[PARAM_LFO_FREQ]);
  const lfoWaveSel = React.useState(initParams[PARAM_LFO_WAVE_TYPE]);

  const masterAmp = React.useState(initParams[PARAM_MASTER_AMP]);

  const store = {
    [PARAM_GEN1_WAVE_TYPE]: wave1Sel,
    [PARAM_GEN2_WAVE_TYPE]: wave2Sel,
    [PARAM_GEN_MIX]: mix,
    [PARAM_FILTER_CUTOFF]: cutoff,
    [PARAM_GEN1_FINE_OFFSET]: gen1FineOffset,
    [PARAM_GEN1_COARSE_OFFSET]: gen1CoarseOffset,
    [PARAM_GEN2_FINE_OFFSET]: gen2FineOffset,
    [PARAM_GEN2_COARSE_OFFSET]: gen2CoarseOffset,
    [PARAM_AMP_ENV_ATTACK]: ampAttack,
    [PARAM_AMP_ENV_DECAY]: ampDecay,
    [PARAM_AMP_ENV_SUSTAIN]: ampSustain,
    [PARAM_AMP_ENV_RELEASE]: ampRelease,
    [PARAM_FILTER_ENV_ATTACK]: filterAttack,
    [PARAM_FILTER_ENV_DECAY]: filterDecay,
    [PARAM_FILTER_ENV_SUSTAIN]: filterSustain,
    [PARAM_FILTER_ENV_RELEASE]: filterRelease,
    [PARAM_DISTORTION]: distortion,
    [PARAM_LFO_AMP]: lfoAmp,
    [PARAM_LFO_FREQ]: lfoFreq,
    [PARAM_LFO_WAVE_TYPE]: lfoWaveSel,
    [PARAM_MASTER_AMP]: masterAmp,
  };

  const selectArgs = (param, values) => {
    return {
      "value": store[param][0],
      "values": values,
      "onValueChanged": (event) => {
        const value = event.target.value;
        store[param][1](value);
        onParamChanged(param, value);
      },
    };
  };

  const knobArgs = (param) => {
    return {
      "value": store[param][0],
      "onValueChanged": (value) => {
        store[param][1](value);
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
