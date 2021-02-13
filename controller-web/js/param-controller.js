function ParamController() {
  const debounceValue = .05;
  const debounceTimeMs = 50;

  const params = {};
  var lastTime = 0;
  var device = null;

  function sendParam(param, value) {
    if (device !== null) {
      console.log("sending", param, value);
      const messages = PARAM_COMMANDS[param](value);
      messages.forEach(message => {
        device.send(message);
      });
    }
  }

  return {
    setDevice: (newDevice) => {
      device = newDevice;
    },
    setParamsBulk: (params) => {
      Object.entries(params).forEach(([param, value]) => {
        sendParam(param, value);
      });
    },
    setParam: (param, value) => {
      const existing = params[param];
      const time = new Date();

      if (
        existing !== value && (
          existing === undefined ||
          time - lastTime >= debounceTimeMs ||
          Math.abs(existing - value) > debounceValue
        )
      ) {
        params[param] = value;
        lastTime = time;
        sendParam(param, value);
      }
    },
  }
}
