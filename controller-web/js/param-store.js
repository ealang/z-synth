function ParamStore(defaultParams) {
  const STORE_KEY = "z-synth-params";

  var params = defaultParams;
  try {
    params = JSON.parse(window.localStorage.getItem(STORE_KEY)) || defaultParams;
  } catch {
  }

  return {
    setParam: (param, value) => {
      params[param] = value;
      window.localStorage.setItem(STORE_KEY, JSON.stringify(params));
    },
    allParams: () => {
      return params;
    },
  };
}
