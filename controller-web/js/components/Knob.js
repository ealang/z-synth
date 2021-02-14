function Knob({
  value=0,
  onValueChanged,
  className=null,
  startValue=0,
  endValue=1,
  startDeg = 200,
  endDeg = -20,
  size = 60,
  lineWidth = 5,
}) {
  const [mouseDown, setMouseDown] = React.useState(false);

  const clampedValue = clamp(value, startValue, endValue);
  const valuePercent = (clampedValue - startValue) / (endValue - startValue);
  const valueDeg = (endDeg - startDeg) * valuePercent + startDeg;

  const knobSensitivity = 50;
  const c = size / 2;
  const circleRad = c - lineWidth * 1.5;

  const nticks = 7;
  const tickMarks = arrayRange(startDeg, endDeg, nticks);

  React.useEffect(() => {
    const rootElem = document.getElementById("root");

    function onMouseMove(e) {
      const delta = e.movementX / knobSensitivity;
      const newValue = clamp(value + delta, 0, 1);
      onValueChanged(newValue);
    }

    function onMouseUp() {
      setMouseDown(false);
    }

    if (mouseDown) {
      rootElem.addEventListener("mousemove", onMouseMove);
      rootElem.addEventListener("mouseup", onMouseUp);
      rootElem.addEventListener("mouseleave", onMouseUp);
    }

    return () => {
      rootElem.removeEventListener("mousemove", onMouseMove);
      rootElem.removeEventListener("mouseup", onMouseUp);
      rootElem.removeEventListener("mouseleave", onMouseUp);
    };

  }, [mouseDown, value]);

  function onMouseDown() {
    setMouseDown(true);
  }

  return (
    <svg onMouseDown={onMouseDown} className={`knob ${className}`} width={size} height={size}>
      <circle strokeWidth={lineWidth} cx={c} cy={c} r={circleRad} />
      <g transform={`translate(${c}, ${c})`}>
        <g transform={`rotate(${-valueDeg})`}>
          <line className="blade" x1={circleRad * 0.2} x2={circleRad} y1={0} y2={0} strokeWidth={lineWidth} />
        </g>
        {
          tickMarks.map(tick => {
            return (
              <g transform={`rotate(${-tick})`}>
                <line className="tick" x1={c - lineWidth / 2} x2={c} y1={0} y2={0} strokeWidth={lineWidth / 4} />
              </g>
            );
          })
        }
      </g>
    </svg>
  );
}
