function App({paramStore, paramController}) {
  const [errorMsg, setErrorMsg] = React.useState("");
  const [devices, setDevices] = React.useState([]);
  const [selectedDevice, setSelectedDevice] = React.useState(null);

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

  function onSyncParams() {
    paramController.setParamsBulk(paramStore.allParams());
  }

  return (
    <div>
      <h1>z-synth</h1>
      <div>
        {errorMsg && <div className="error">Error initializing Midi: {errorMsg}</div>}
        {!errorMsg && <MidiDeviceSelector
          devices={devices}
          selectedDevice={selectedDevice}
          onSelectDevice={onDeviceChanged} />}
      </div>
      <div>
        <ParamControls initParams={paramStore.allParams()} onParamChanged={onParamChanged} />
        <button onClick={onSyncParams}>Sync Params</button>
      </div>
    </div>
  );
}
