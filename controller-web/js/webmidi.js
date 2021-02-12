function webMidiSubscribe(onDevicesUpdate, onError) {
  function connect() {
    if (!navigator.requestMIDIAccess) {
      onError("WebMIDI is not supported by this browser");
    } else {
      navigator.requestMIDIAccess().then(midiReady, onError);
    }
  }

  function midiReady(midi) {
    midi.addEventListener('statechange', (event) => internalOnDevicesUpdate(event.target));
    internalOnDevicesUpdate(midi);
  }

  function internalOnDevicesUpdate(midi) {
    const outputDevices = [];
    const outputs = midi.outputs.values();
    for (let output = outputs.next(); output && !output.done; output = outputs.next()) {
      outputDevices.push(output.value);
    }

    onDevicesUpdate(outputDevices);
  }

  connect();
}
