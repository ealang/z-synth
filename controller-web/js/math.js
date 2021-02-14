function clamp(v, min, max) {
  return Math.min(Math.max(v, min), max);
}

function arrayRange(start, end, n) {
  const step = (end - start) / (n - 1);
  const values = [];
  var cur = start;
  for (var i = 0; i < n; ++i) {
    values.push(cur);
    cur += step
  }
  return values;
}
