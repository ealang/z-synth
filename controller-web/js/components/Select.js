function Select({values, value, onValueChanged}) {
  return (
    <select value={value} onChange={onValueChanged}>
      {values.map(item => {
        return <option value={item}>{item}</option>;
      })}
    </select>
  );
}
