function ParamStore(defaultParams) {
  const STORE_KEY = "z-synth-params";

  function load() {
    var params = {...defaultParams};
    try {
      params = JSON.parse(window.localStorage.getItem(STORE_KEY)) || params;
    } catch {
    }
    return params;
  }

  function save(params) {
    window.localStorage.setItem(STORE_KEY, JSON.stringify(params));
  }

  var params = load();

  return {
    setParam: (param, value) => {
      params[param] = value;
      save(params);
    },
    allParams: () => {
      return params;
    },
    reset: () => {
      params = {...defaultParams};
      console.log(params);
      save(params);
    }
  };
}
