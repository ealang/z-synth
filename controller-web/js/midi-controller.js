function MidiController() {
  const debounceValue = .05;
  const debounceTimeMs = 50;

  var lastTime = 0;
  var device = null;

  function sendParam(param, value) {
    if (device !== null) {
      const cmdGenerator = PARAM_COMMANDS[param];
      if (cmdGenerator) {
        const messages = cmdGenerator(value);
        console.log("Sending", param, value);
        messages.forEach(message => {
          device.send(message);
        });
      } else {
        console.error("Ignoring unknown param", param);
      }
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
      sendParam(param, value);
    },
  }
}
