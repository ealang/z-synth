function App({paramStore, paramController}) {
  const [errorMsg, setErrorMsg] = React.useState("");
  const [devices, setDevices] = React.useState([]);
  const [selectedDevice, setSelectedDevice] = React.useState(null);
  const [instance, setInstance] = React.useState(null);

  React.useEffect(() => {
    webMidiSubscribe(setDevices, setErrorMsg);
  }, []);

  function onParamChanged(param, value) {
    paramController.setParam(param, value);
    paramStore.setParam(param, value);
  }

  function onDeviceChanged(device) {
    paramController.setDevice(device);
    setSelectedDevice(device);
  }

  function onSendAll() {
    paramController.setParamsBulk(paramStore.allParams());
  }

  function onResetParams() {
    paramStore.reset();
    setInstance(new Date());
    onSendAll();
  }

  return (
    <div>
      <h1>z-synth</h1>
      <div className="app-device">
        {errorMsg && <div className="error">Error initializing Midi: {errorMsg}</div>}
        {!errorMsg && <MidiDeviceSelector
          devices={devices}
          selectedDevice={selectedDevice}
          onSelectDevice={onDeviceChanged} />}
        <div>
          <button onClick={onSendAll}>Send All</button>
          <button onClick={onResetParams}>Reset</button>
        </div>
      </div>
      <div className="app-params">
        <ParamControls key={instance} initParams={paramStore.allParams()} onParamChanged={onParamChanged} />
      </div>
    </div>
  );
}
