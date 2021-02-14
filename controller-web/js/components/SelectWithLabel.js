function SelectWithLabel({label, ...args}) {
  return (
    <div className="flex-container-column select-label-container">
      <div>
        <Select {...args} />
      </div>
      <div>{label}</div>
    </div>
  );
}
