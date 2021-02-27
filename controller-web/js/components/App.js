function App({paramStore, presetStore, midiController}) {
  const [errorMsg, setErrorMsg] = React.useState("");
  const [devices, setDevices] = React.useState([]);
  const [selectedDevice, setSelectedDevice] = React.useState(null);

  const [params, setParams] = React.useState(paramStore.allParams());
  const [presets, setPresets] = React.useState(presetStore.presets());
  const [newPresetName, setNewPresetName] = React.useState("");

  React.useEffect(() => {
    webMidiSubscribe(setDevices, setErrorMsg);
  }, []);

  function onParamChanged(param, value) {
    midiController.setParam(param, value);
    setParams(paramStore.setParam(param, value));
  }

  function onDeviceChanged(device) {
    midiController.setDevice(device);
    setSelectedDevice(device);
  }

  function onSendAll(params) {
    midiController.setParamsBulk(params);
  }

  function onResetParams(params) {
    const newParams = paramStore.reset();
    setParams(newParams);
    onSendAll(newParams);
  }

  function onAddPreset() {
    if (newPresetName) {
      setPresets(
        presetStore.addPreset(newPresetName, params),
      );
      setNewPresetName("");
    }
  }

  function onOverwritePreset(i) {
    setPresets(
      presetStore.overwritePreset(i, params)
    );
  }

  function onLoadPreset(i) {
    const newParams = paramStore.replaceParams(presets[i][1]);
    setParams(newParams);
    onSendAll(newParams);
  }

  function onDeletePreset(i) {
    setPresets(
      presetStore.deletePreset(i),
    );
  }

  return (
    <div>
      <h1 className="app-section">z-synth</h1>
      <div className="app-device app-section">
        {errorMsg && <div className="error">Error initializing Midi: {errorMsg}</div>}
        {!errorMsg && <MidiDeviceSelector
          devices={devices}
          selectedDevice={selectedDevice}
          onSelectDevice={onDeviceChanged} />}
        <div>
          <button onClick={() => onSendAll(paramStore.allParams())}>Send All</button>
          <button onClick={onResetParams}>Reset</button>
        </div>
      </div>
      <div className="app-presets app-section">
        {
          presets.map(([presetName, _], i) => {
            return <div className="preset-container">
              <button className="load-button" onClick={() => onLoadPreset(i)}>{presetName}</button>
              <button onClick={() => onOverwritePreset(i)}>	&#x1f4be;</button>
              <button onClick={() => onDeletePreset(i)}> &#x2716; </button>
            </div>;
          })
        }
        <div className="new-preset-name">
          <input type="text" placeholder="New preset name" value={newPresetName} onChange={e => setNewPresetName(e.target.value)}></input>
          <button onClick={onAddPreset}>Add</button>
        </div>
      </div>
      <div className="app-section">
        <ParamControls params={params} onParamChanged={onParamChanged} />
      </div>
    </div>
  );
}
