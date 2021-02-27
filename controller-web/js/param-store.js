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

  var _params = load();

  return {
    setParam: (param, value) => {
      _params = {..._params, [param]: value};
      save(_params);
      return _params;
    },
    replaceParams: (params) => {
      _params = {...params};
      save(_params);
      return _params;
    },
    allParams: () => {
      return _params;
    },
    reset: () => {
      _params = {...defaultParams};
      save(_params);
      return _params;
    }
  };
}
