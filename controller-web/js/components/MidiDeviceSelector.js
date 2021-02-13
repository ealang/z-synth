function MidiDeviceSelector({devices, selectedDevice, onSelectDevice}) {
  function deviceFromId(deviceId) {
    const i = devices.findIndex(_ => _.id === deviceId);
    if (i === -1) {
      return null;
    }
    return devices[i];
  }

  if (selectedDevice !== null && !deviceFromId(selectedDevice.id)) {
    onSelectDevice(null);
  } else if (selectedDevice === null && devices.length > 0) {
    onSelectDevice(devices[0]);
  }

  return (
    <select value={selectedDevice && selectedDevice.id} onChange={(e) => onSelectDevice(deviceFromId(e.target.value))}>
      {devices.map(device => {
        return <option value={device.id}>{device.name}</option>;
      })}
    </select>
  );
}
