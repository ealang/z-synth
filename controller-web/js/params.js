const NRPN_MSB_VALUE = 0x10;

const PARAM_GEN1_WAVE_TYPE     = 0x00;
const PARAM_GEN2_WAVE_TYPE     = 0x01;
const PARAM_GEN_MIX            = 0x02;
const PARAM_FILTER_CUTOFF      = 0x03;
const PARAM_GEN1_OFFSET        = 0x04;
const PARAM_GEN2_OFFSET        = 0x05;
const PARAM_AMP_ENV_ATTACK     = 0x06;
const PARAM_AMP_ENV_DECAY      = 0x07;
const PARAM_AMP_ENV_SUSTAIN    = 0x08;
const PARAM_AMP_ENV_RELEASE    = 0x09;
const PARAM_FILTER_ENV_ATTACK  = 0x0A;
const PARAM_FILTER_ENV_DECAY   = 0x0B;
const PARAM_FILTER_ENV_SUSTAIN = 0x0C;
const PARAM_FILTER_ENV_RELEASE = 0x0D;
const PARAM_DISTORTION         = 0x0E;
const PARAM_LFO_AMP            = 0x0F;
const PARAM_LFO_FREQ           = 0x10;
const PARAM_LFO_WAVE_TYPE      = 0x11;
const PARAM_MASTER_AMP         = 0x12;

const PARAM_WAVE_OPTIONS = ["square", "sine", "triangle", "noise", "saw"];

const DEFAULT_PARAM_VALUES = {
  [PARAM_GEN1_WAVE_TYPE]: PARAM_WAVE_OPTIONS[0],
  [PARAM_GEN2_WAVE_TYPE]: PARAM_WAVE_OPTIONS[1],
  [PARAM_GEN_MIX]: 0.5,
  [PARAM_FILTER_CUTOFF]: 0.5,
  [PARAM_GEN1_OFFSET]: 0.5,
  [PARAM_GEN2_OFFSET]: 0.5,

  [PARAM_AMP_ENV_ATTACK]: 0.1,
  [PARAM_AMP_ENV_DECAY]: 0.1,
  [PARAM_AMP_ENV_SUSTAIN]: 0.5,
  [PARAM_AMP_ENV_RELEASE]: 0.1,

  [PARAM_FILTER_ENV_ATTACK]: 0.1,
  [PARAM_FILTER_ENV_DECAY]: 0.1,
  [PARAM_FILTER_ENV_SUSTAIN]: 0.5,
  [PARAM_FILTER_ENV_RELEASE]: 0.1,

  [PARAM_DISTORTION]: 0,
  [PARAM_LFO_AMP]: 0.5,
  [PARAM_LFO_FREQ]: 0.1,
  [PARAM_LFO_WAVE_TYPE]: PARAM_WAVE_OPTIONS[1],
  [PARAM_MASTER_AMP]: 0.8,
};

const PARAM_COMMANDS = (() => {
  const CONTROL_CHANGE = 0xB0;
  const CONTROL_DATA_MSB = 0x06;
  const CONTROL_NRPN_MSB = 0x63;
  const CONTROL_NRPN_LSB = 0x62;

  function linear(start = 0, end = 127) {
    return (value) => {
      return (value * (end - start)) | 0;
    };
  }

  function indexed(array) {
    return (value) => {
      const i = array.indexOf(value);
      if (i === -1) {
        console.error("Invalid value", array, value);
        return 0;
      }
      return i;
    };
  }

  function nrpn(lsb, rangeFunc) {
    return (value) => {
      return [
        [CONTROL_CHANGE, CONTROL_NRPN_MSB, NRPN_MSB_VALUE],
        [CONTROL_CHANGE, CONTROL_NRPN_LSB, lsb],
        [CONTROL_CHANGE, CONTROL_DATA_MSB, rangeFunc(value)],
      ];
    };
  }

  return {
    [PARAM_GEN1_WAVE_TYPE]: nrpn(PARAM_GEN1_WAVE_TYPE, indexed(PARAM_WAVE_OPTIONS)),
    [PARAM_GEN2_WAVE_TYPE]: nrpn(PARAM_GEN2_WAVE_TYPE, indexed(PARAM_WAVE_OPTIONS)),
    [PARAM_GEN_MIX]: nrpn(PARAM_GEN_MIX, linear()),
    [PARAM_FILTER_CUTOFF]: nrpn(PARAM_FILTER_CUTOFF, linear()),
    [PARAM_GEN1_OFFSET]: nrpn(PARAM_GEN1_OFFSET, linear()),
    [PARAM_GEN2_OFFSET]: nrpn(PARAM_GEN2_OFFSET, linear()),
    [PARAM_AMP_ENV_ATTACK]: nrpn(PARAM_AMP_ENV_ATTACK, linear()),
    [PARAM_AMP_ENV_DECAY]: nrpn(PARAM_AMP_ENV_DECAY, linear()),
    [PARAM_AMP_ENV_SUSTAIN]: nrpn(PARAM_AMP_ENV_SUSTAIN, linear()),
    [PARAM_AMP_ENV_RELEASE]: nrpn(PARAM_AMP_ENV_RELEASE, linear()),
    [PARAM_FILTER_ENV_ATTACK]: nrpn(PARAM_FILTER_ENV_ATTACK, linear()),
    [PARAM_FILTER_ENV_DECAY]: nrpn(PARAM_FILTER_ENV_DECAY, linear()),
    [PARAM_FILTER_ENV_SUSTAIN]: nrpn(PARAM_FILTER_ENV_SUSTAIN, linear()),
    [PARAM_FILTER_ENV_RELEASE]: nrpn(PARAM_FILTER_ENV_RELEASE, linear()),
    [PARAM_DISTORTION]: nrpn(PARAM_DISTORTION, linear()),
    [PARAM_LFO_AMP]: nrpn(PARAM_LFO_AMP, linear()),
    [PARAM_LFO_FREQ]: nrpn(PARAM_LFO_FREQ, linear()),
    [PARAM_LFO_WAVE_TYPE]: nrpn(PARAM_LFO_WAVE_TYPE, indexed(PARAM_WAVE_OPTIONS)),
    [PARAM_MASTER_AMP]: nrpn(PARAM_MASTER_AMP, linear()),
  };
})();
