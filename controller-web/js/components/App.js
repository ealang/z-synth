function Sender({device}) {
  const CONTROL_CHANGE = 0xB0;
  const CONTROL_DATA_MSB = 0x06;
  const CONTROL_DATA_LSB = 0x26;
  const CONTROL_NRPN_MSB = 0x63;
  const CONTROL_NRPN_LSB = 0x62;

  function onSend() {
    if (device) {
      // device.send([0x90, 40, 127]);

      device.send([CONTROL_CHANGE, CONTROL_NRPN_MSB, 0x13]);
      device.send([CONTROL_CHANGE, CONTROL_NRPN_LSB, 0x37]);
      device.send([CONTROL_CHANGE, CONTROL_DATA_MSB, 40]);
    }
  }

  return (
    <div>
      <button onClick={onSend}>Send</button>
    </div>
  );
}

function App() {
  const [errorMsg, setErrorMsg] = React.useState("");
  const [devices, setDevices] = React.useState([]);
  const [selectedDevice, setSelectedDevice] = React.useState(null);

  React.useEffect(() => {
    webMidiSubscribe(setDevices, setErrorMsg);
  }, []);

  function onSelectDevice(device) {
    console.log("got device", device && device.name);
    setSelectedDevice(device);
  }

  return (
    <div>
      <div>
        <h1>Device</h1>
        {errorMsg && <div className="error">Error initializing Midi: {errorMsg}</div>}
        {!errorMsg && <MidiDeviceSelector
          devices={devices}
          selectedDevice={selectedDevice}
          onSelectDevice={onSelectDevice} />}
      </div>
      {selectedDevice && <Sender device={selectedDevice} />}
    </div>
  );
}
