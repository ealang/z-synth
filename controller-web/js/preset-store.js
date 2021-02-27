function PresetStore() {
  const STORE_KEY = "z-synth-presets";

  function load() {
    var presets = [];
    try {
      presets = JSON.parse(window.localStorage.getItem(STORE_KEY)) || presets;
    } catch {
    }
    return presets;
  }

  function save(presets) {
    window.localStorage.setItem(STORE_KEY, JSON.stringify(presets));
  }

  var presets = load();

  return {
    addPreset: (name, value) => {
      presets = [...presets, [name, value]];
      save(presets);
      return presets;
    },
    presets: () => presets,
    overwritePreset: (index, value) => {
      presets = [...presets];
      presets[index][1] = value;
      save(presets);
      return presets;
    },
    deletePreset: (index) => {
      presets = [...presets];
      presets.splice(index, 1);
      save(presets);
      return presets;
    }
  };
}
