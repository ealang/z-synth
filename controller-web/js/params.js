const PARAM_MIX = "mix";
const PARAM_CUTOFF = "cutoff";
const PARAM_WAVE = "wave";

const PARAM_WAVE_OPTIONS = ["square", "sine", "triangle", "noise"];

const DEFAULT_PARAM_VALUES = {
  [PARAM_MIX]: 0.5,
  [PARAM_CUTOFF]: 0.3,
  [PARAM_WAVE]: PARAM_WAVE_OPTIONS[0],
}

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

  function nrpn(msb, lsb, rangeFunc = identity) {
    return (value) => {
      return [
        [CONTROL_CHANGE, CONTROL_NRPN_MSB, msb],
        [CONTROL_CHANGE, CONTROL_NRPN_LSB, lsb],
        [CONTROL_CHANGE, CONTROL_DATA_MSB, rangeFunc(value)],
      ];
    };
  }

  return {
    [PARAM_MIX]: nrpn(0x13, 0x39, linear()),
    [PARAM_CUTOFF]: nrpn(0x13, 0x38, linear()),
    [PARAM_WAVE]: nrpn(0x13, 0x37, indexed(PARAM_WAVE_OPTIONS)),
  };
})();
