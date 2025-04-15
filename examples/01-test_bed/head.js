var appName = document.title;
let OSName = client_system();
var is_windows = false;
if (OSName === "Windows") is_windows = true;

function my_confirm(message) {
  var iframe = document.createElement("IFRAME");
  iframe.setAttribute("src", 'data:text/plain,');
  document.documentElement.appendChild(iframe);
  if (window.frames[0].window.confirm(message)) {
    return true;
  } else {
    return false;
  }
}
var host = "",
  user = "";
App.prototype.prompt = async function() {
  if (is_windows) {
    if (host === "") host = await env.get("COMPUTERNAME");
    if (user === "") user = await env.get("USERNAME");
  } else {
    if (host === "") host = await env.get("HOSTNAME");
    if (user === "") user = await env.get("USER");
  }
  if (host === "" || host.startsWith("No value found for")) {
    host = await app.system("hostname");
  }
  //console.log(fs.cwd);
  return `${document.title} - ${user}@${host}:${fs.cwd.fromHtmlEntities()}`; //:${await fs.current_path()}`;
}
async function my_app_title() {
  prmpt = await app.prompt();
  app.set_title(prmpt);
}
// Fonctionnement spécifique a webview-app
if (typeof app.set_title === "function") {
  // Prevent the app to load multiple times with the help of a BroadcastChannel
  const bc = new BroadcastChannel(appName);

  function clean_exit() {
    console.log("clean_exit");
    if (cmd_file.value != "") {
      console.log("set cmd_file");
      localStorage.setItem("jfile", cmd_file.value);
    }
    bc.close();
    app.exit();
  }

  function only_one_loaded_instance() {
    bc.postMessage(appName);
    bc.onmessage = (event) => {
      if (event.data === appName) bc.postMessage("already loaded");
      else if (event.data === "already loaded") app.exit();
    }
  }
  only_one_loaded_instance();
  // Eventually clear all localStorage values
  // localStorage.clear();
  // Eventually display the configuration data to the console
  //for (const key of Object.keys(localStorage)) { console.log("onload: "+key+": "+localStorage.getItem(key)); }
  // Set window title, icon and minimal dimensions
  my_app_title();
  app.set_icon("app.ico");
  app.set_size(650, 220, 1);
  /*.then((res) => {
    console.log(JSON.stringify(res));
  });*/
}

function exit_on_esc() {
  if (event.keyCode === 27) clean_exit();
}
// The 2 following functions are in case of Ctrl-P or win.printdlg()
window.onafterprint = async function() {
  // Wait a little ...
  await new Promise(r => setTimeout(r, 400));
  // And re-enable esc_on_exit
  document.addEventListener("keyup", exit_on_esc);
  console.log("Printing completed...");
}
window.onbeforeprint = function() {
  // Disable esc_on_exit which may interfer with esc on alert
  document.removeEventListener("keyup", exit_on_esc);
  console.log("Printing starting...");
}
String.prototype.toHtmlEntities = function() {
  return this.replace(/./gm, function(s) {
    ord = s.charCodeAt(0);
    if (ord > 31 && ord < 127) return s;
    else return (s.match(/[a-z0-9\s]+/i)) ? s : "&#x" + ord.toString(16) + ";";
  });
};
String.prototype.fromHtmlEntities = function() {
  return this.replace(/&#\d+;/gm, function(s) {
    return String.fromCharCode(s.match(/\d+/gm)[0]);
  })
};
async function about() {
  var appInfo = app.info.fromHtmlEntities();
  console.log("app.info=[" + app.info + "]");
  console.log("appInfo =[" + appInfo + "]");
  // Disable esc_on_exit which may interfer with esc on alert
  document.removeEventListener("keyup", exit_on_esc);
  await gui.msgbox((appInfo).toHtmlEntities());
  // Re-enable esc_on_exit
  await new Promise(r => setTimeout(r, 400));
  document.addEventListener("keyup", exit_on_esc);
}

function cls() {
  output.innerHTML = "";
  output.scrollTop = output.scrollHeight;
}

function print(s = "", scrollToBottom = true) {
  output.innerHTML += s;
  if (scrollToBottom) output.scrollTop = output.scrollHeight;
}

function println(s = "", scrollToBottom = true) {
  output.innerHTML += s + "<br/>";
  if (scrollToBottom) output.scrollTop = output.scrollHeight;
}

function fl_sort(a, b) {
  if (a.startsWith('*')) a1 = a.substring(1);
  else a1 = a;
  if (b.startsWith('*')) b1 = b.substring(1);
  else b1 = b;
  return a1 > b1;
}
var func_list = [],
  func_help = [];
var var_help = [];
(async () => {
  if (func_list.length === 0) {
    var help_arr = await app.help("json");
    //console.log(help_arr);
    help_arr.forEach((elt) => {
      sh = "";
      sl = "";
      if (Object.hasOwn(elt, "vars")) {
        //console.log(JSON.stringify(elt.vars.length));
        elt.vars.forEach((vl) => {
          var vline = "";
          if (vl.readonly) vline += '-';
          else vline += ' ';
          vline += `${vl.object}.${vl.var_name}: ${vl.desc}`;
          var_help.push(vline);
          //console.log(vline);
        });
      } else {
        if (Object.hasOwn(elt, "async") && elt["async"]) {
          sh += '*';
          sl += "await ";
        } else {
          sh += ' ';
        }
        sh += elt.name + ": ";
        sl += elt.name + "() // ";
        com = "";
        if (Object.hasOwn(elt, "param") && elt["param"] !== 0) {
          p = elt["param"];
          vp = Math.abs(p);
          if (p >= 9999) {
            com += "at least 1 parameter";
          } else if (p <= -9999) {
            com += "indefinite number of parameters";
          } else {
            if (p > 0) com += "needs " + vp;
            else if (p < 0) com += "accepts up to " + vp;
            com += " parameter";
            if (vp > 1) com += "s";
          }
          if (Object.hasOwn(elt, "desc")) com += ", ";
        }
        if (Object.hasOwn(elt, "desc")) {
          com += elt["desc"];
        }
        sh += com;
        sl += com;
        func_help.push(sh);
        func_list.push(sl);
      }
    });
    func_help.sort(fl_sort);
    func_list.sort(fl_sort);
    //console.log(JSON.stringify(func_list, null, 2));
  }
})();
async function readfile(filename, obj = output) {
  window.fs.read(filename).then(txt => {
    //txt = decodeEntities(txt);
    if (obj instanceof HTMLElement) obj.innerHTML += txt.replace(/\n/g, "<br/>");
    console.log(txt);
  });
}
async function home_api(url = "https://home.dplalanne.fr/free/index.php?hello&name=Denis&json") {
  const res = await fetch(url);
  const js = await res.json();
  output.innerHTML = `Response from: '${url}' :\nYour name: ${js.your_name}\nMy name: ${js.my_name}`;
  console.log(JSON.stringify(js));
}
async function mygithub() {
  const response = await fetch("https://api.github.com/users/2nil-web", {
    mode: "cors"
  });
  const data = await response.json();
  rep = data.repos_url;
  println(rep);
  console.log(data);
}

function reqListener() {
  console.log(JSON.parse(this.responseText));
}

function http_query(url = "https://api.github.com/users/2nil-web") {
  const req = new XMLHttpRequest();
  req.addEventListener("load", reqListener);
  req.open("GET", url);
  // req.setRequestHeader("Access-Control-Allow-Origin", "*");
  // req.setRequestHeader("Authorization", "Basic " + btoa("id:password"));
  req.send();
}