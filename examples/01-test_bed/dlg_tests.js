(async () => {
  // Disable esc_on_exit which may interfer with esc on msg_box
  document.removeEventListener("keyup", exit_on_esc);
  if (confirm("Test file opening and file saving dialogs")) {
    var _opendlg, _savedlg;
    if (OSName === "Windows") {
      _opendlg = win_opendlg;
      _savedlg = win_savedlg;
    }
    if (OSName === "Linux") {
      _opendlg = lin_opendlg;
      _savedlg = lin_savedlg;
    }
    console.log("OS: " + OSName);
    file = await _opendlg();
    console.log("OPEN: [" + file + "]");
    if (file != "") {
      println("Opening file: " + file);
      console.log("Opening file: " + file);
    }
    file = await _savedlg();
    console.log("SAVE: [" + file + "]");
    if (file != "") {
      println("Saving to file: " + file);
      console.log("Saving to file: " + file);
    }
  }
  // Re-enable esc_on_exit
  await new Promise(r => setTimeout(r, 400));
  document.addEventListener("keyup", exit_on_esc);
  jscript_line.disabled = false;
  cmd_line.focus();
})()