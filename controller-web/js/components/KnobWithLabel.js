function KnobWithLabel({label, ...args}) {
  return (
    <div className="flex-container-column knob-label-container">
      <Knob {...args} />
      <div>{label}</div>
    </div>
  );
}
