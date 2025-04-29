function setDarkMode(set, record = true) {
  if (typeof win !== "undefined" && typeof win.dark_bar === "function") win.dark_bar(set);
  if (record) localStorage.setItem("stat_dark", set);
  if (typeof app.set_icon === "function") {
    icfile = "app";
    if (set) icfile += "_dark";
    icfile += ".";
    if (client_system() == "Windows") icfile += "ico";
    else icfile += "svg";
    console.log("Changing window bar icon to " + icfile);
    app.set_icon(icfile);
  }
  darkMode.checked = set;
  lightMode.checked = !set;
}

function tokei_to_table(s) {
  var vs = s.split("\n");
  var tbl = "<table><tr><td>Language</td><td>Files</td><td>Lines</td><td>Code</td><td>Notes</td><td>Blanks</td></tr>\n";
  const cre = /  */g,
    rep = "</td><td>",
    sol = "<tr><td>",
    eol = "</td></tr>";
  for (i = 3; i < vs.length; i++) {
    l = vs[i].trim();
    if (!l.startsWith("-----") && !l.startsWith("=====") && !l.length !== 0) {
      var tr = "";
      if (l.startsWith("C Header")) {
        tr = sol + "C&nbsp;Header" + l.substring(8).replace(cre, rep) + eol;
      } else if (l.startsWith("Plain Text")) {
        tr = sol + "Plain&nbsp;Text" + l.substring(10).replace(cre, rep) + eol;
      } else if (l.startsWith("|-")) {
        tr = sol + "<small>&nbsp;&nbsp;&nbsp;&nbsp;&#10551;<i>" + l.substring(3).replace(cre, rep) + "</small>" + eol;
      } else if (l.startsWith("(Total)")) {
        tr = sol + "<i>Subtotal</i></td><td>" + l.substring(7).replace(cre, rep) + eol;
      } else if (l.startsWith("Total")) {
        tr = sol + "Total" + l.substring(5).replace(cre, rep) + eol;
      } else {
        tr = sol + "" + l.replace(cre, rep) + eol;
      }
      if (tr !== sol + eol) tbl += tr + "\n";
    }
  }
  //  console.log(tbl);
  return tbl;
}
// Compute file age in milliseconds
async function file_age(fname) {
  st = JSON.parse(await fs.stat(fname, true));
  file_time = new Date(st.last_write).getTime();
  now = Date.now();
  file_age = (now - file_time) / 3600000;
  return false;
}
// If tokei.txt is older than 24 hours return true, else false
async function need_compute() {
  if ((await fs.exists("tokei.txt")) && (await file_age("tokei.txt")) / 3600000 < 24) return false;
  return true;
}
async function load(force_compute = false) {
  var tokei_res;
  if (force_compute || (await need_compute())) {
    console.log("Computing tokei_res");
    toggleDark.style.display = "none";
    dynaDiv.style.display = "none";
    // Spinner
    spinCenter.style.display = "block";
    spinner.src = "spinner" + (Math.floor(Math.random() * 5)) + ".svg";
    var actual_path = await fs.current_path();
    await fs.current_path("../..");
    tokei_param = " --hidden --exclude winerrv.h Makefile header.mk rules.mk src scripts assets shortcuts examples/01-test_bed examples/02-stats examples/03-webcrypto examples/04-curves examples/05-azimuth examples/06-backup examples/init_geom examples/js_include"; // > "+actual_path+"/stats.json";
    var tokei_res = "";
    if (client_system() === "Windows") {
      //await env.add_path(await env.get("LOCALAPPDATA")+"/Microsoft/WinGet/Packages/XAMPPRocky.Tokei_Microsoft.Winget.Source_8wekyb3d8bbwe");
      await env.add_path("C:\\UnixTools\\bin");
      tokei_cmd = await app.shell("where tokei");
      console.log(tokei_cmd);
      if (!tokei_cmd.startsWith("Information : impossible de trouver des fichiers pour ")) tokei_res = await app.shell(tokei_cmd.trim() + tokei_param);
    } else {
      await env.add_path(await env.get("HOME") + "/.cargo/bin");
      tokei_cmd = await app.system("which tokei");
      if (!tokei_cmd.startsWith("which: no ")) tokei_res = await app.system(tokei_cmd.trim() + tokei_param);
    }
    await fs.current_path(actual_path);
    await fs.write("tokei.txt", tokei_res);
    await new Promise(r => setTimeout(r, 1000));
  } else {
    console.log("Reading tokei_res");
    tokei_res = await fs.read("tokei.txt");
  }
  if (tokei_res !== "") {
    dynaDiv.innerHTML = tokei_to_table(tokei_res);
  } else {
    dynaDiv.innerHTML = "Unable to run tokei to get stats";
  }
  spinCenter.style = "display:none";
  toggleDark.style.display = "block";
  dynaDiv.style.display = "block";
  if (typeof app.set_title === "function") {
    app.set_title(document.title);
    app.set_icon("app.ico");
    document.addEventListener("keyup", () => {
      if (event.keyCode === 27) app.exit();
    });
  }
  var dark_mode = (localStorage.getItem("stat_dark") === "true");
  if (dark_mode) {
    setDarkMode(true, false);
  }
}