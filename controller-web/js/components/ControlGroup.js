function ControlGroup({label, children}) {
  return (
    <div className="control-group">
      <h3 className="control-group-label">{label}</h3>
      <div className="control-group-contents flex-container-row">
        {children}
      </div>
    </div>
  );
}
