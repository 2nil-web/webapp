var appName = document.title;
let OSName = client_system();
var is_windows = false;
if (OSName === "Windows") is_windows = true;
if (false) { // Pour tracer
  function getLine(offset) {
    var stack = new Error().stack.split('\n'),
      line = stack[(offset || 1) + 1].split(':');
    return parseInt(line[line.length - 2], 10);
  }
  Object.defineProperty(window, '__LINE__', {
    get: function() {
      return getLine(2);
    }
  });
  var ntrc = 1;

  function trc(l, s) {
    console.log("Line n°" + l + ", trace call n°" + ntrc++ + ", message: " + s);
  }
}

function getItemOrDefault(itemId, defVal, msg = "") {
  itemVal = localStorage.getItem(itemId);
  if (itemVal === null || itemVal === "") {
    itemVal = defVal;
    localStorage.setItem(itemId, itemVal);
  }
  return itemVal;
};

function getBoolItemOrDefault(itemId, defVal) {
  return (getItemOrDefault(itemId, defVal, "boolean ") === "true");
};

function exit_on_esc() {
  if (event.keyCode === 27) app.exit();
}
var HostName = "";
(async () => {
  // Eventually clear all localStorage values
  //localStorage.clear();
  // Eventually display the configuration data to the console
  //for (const key of Object.keys(localStorage)) { console.log("onload "+key, localStorage.getItem(key)); }
  if (typeof app.set_title === "function") {
    app.set_title(appName);
    app.set_icon("app.ico");
    // Fixed size
    //app.set_size(704, 150, 0);
    HostName = await env.get("COMPUTERNAME");
  }
  // Get all the backup targets
  const allBak = document.getElementsByTagName("bak-ln");
  for (i = 0; i < allBak.length; i++) {
    chkd = getBoolItemOrDefault(`${appName}.bak-ln.${i}.checked`, false);
    //console.log(`${appName}.bak-ln.${i}.checked: ${chkd}`);
  }
})();
window.addEventListener('load', onloadcb);
document.addEventListener("keyup", exit_on_esc);
async function about() {
  console.log(app.info);
  // Disable esc_on_exit which may interfer with esc on alert
  document.removeEventListener("keyup", exit_on_esc);
  /*if (is_windows)*/
  await gui.msgbox(app.info);
  //else window.alert(app.info);
  // Re-enable esc_on_exit
  await new Promise(r => setTimeout(r, 100));
  document.addEventListener("keyup", exit_on_esc);
}

function getbaklncount() {
  var blist = document.getElementById("backup-list");
  var bklns = blist.getElementsByTagName("bak-ln");
  var cntbln = 0;
  for (i = 0; i < bklns.length; i++) {
    if (bklns[i].getAttribute("host") === HostName) cntbln++
  }
  return cntbln;
}
var no_more_sort = false;
async function sortBakLn() {
  if (no_more_sort) return;
  var table;
  nwait = 0;
  // Wait for the table to be full
  for (;;) {
    table = document.getElementById("BackupTable");
    if (table !== null) {
      //console.log(table.rows.length, getbaklncount());
      if (table.rows.length === getbaklncount()) break;
      await new Promise(r => setTimeout(r, 200));
      if (table.rows.length > getbaklncount()) window.location.reload();
      if (nwait++ > 10) {
        await new Promise(r => setTimeout(r, 400));
        window.location.reload();
        break;
      }
    } else {
      await new Promise(r => setTimeout(r, 800));
    }
  }
  for (ns = 0; ns < 2; ns++) {
    table = document.getElementById("BackupTable");
    //console.log("Sorting", table.rows.length, getbaklncount());
    var rows, switching, i, x, y, shouldSwitch;
    switching = true;
    // Make a loop that will continue until no switching has been done:
    while (switching) {
      // Start by saying: no switching is done:
      switching = false;
      rows = table.rows;
      // Loop through all table rows
      for (i = 0; i < (rows.length - 1); i++) {
        // Start by saying there should be no switching:
        shouldSwitch = false;
        // Get the two elements you want to compare, one from current row and one from the next:
        x = rows[i].getElementsByTagName("td");
        y = rows[i + 1].getElementsByTagName("td");
        // Check if the two rows should switch place:
        if (x[3].title < y[3].title && x[1].title > y[1].title) {
          // If so, mark as a switch and break the loop:
          shouldSwitch = true;
          break;
        }
      }
      if (shouldSwitch) {
        // If a switch has been marked, make the switch and mark that a switch has been done:
        rows[i].parentNode.insertBefore(rows[i + 1], rows[i]);
        switching = true;
      }
    }
    await new Promise(r => setTimeout(r, 800));
  }
  no_more_sort = true;
  /*
    for (i = 0; i < (rows.length - 1); i++) {
      console.log(rows[i]);
    }*/
}
class BakLn extends HTMLElement {
  static observedAttributes = ["src", "dst"];
  constructor() {
    super();
  }
  static bakLnCbCount = 0;
  create = function() {
    var host = this.getAttribute("host");
    if (host !== null && host !== HostName) return;
    var backupTable;
    // Créé une nouvelle table si le 1er enfant du père n'en est pas une.
    if (typeof this.parentNode.firstChild.tagName === 'undefined' || this.parentNode.firstChild.tagName.toLowerCase() !== 'table') {
      //console.log("Creating a new table");
      backupTable = document.createElement("table");
      backupTable.setAttribute("id", "BackupTable");
      backupTable.style = "border-collapse:separate;";
      backupTable.style = "border-spacing: 4px 4px;";
      this.parentNode.prepend(backupTable);
    } else backupTable = this.parentNode.firstChild;
    // Add a row
    //console.log(Date.now().toString(36)+", "+Date.now().valueOf()+", "+new Date().valueOf());
    var tr = this.parentNode.firstChild.insertRow(-1);
    tr.style = "outline:thin solid; user-select: none";
    //tr.style="user-select: none;";
    function addTdObj(obj, title) {
      var td = document.createElement("td");
      obj.style = "display: block;"; // Pour étirer l'objet sur toute la largeur de la cellule
      td.appendChild(obj);
      tr.appendChild(td);
      return td;
    }

    function addTdLabel(txt) {
      var lab = document.createElement("label");
      lab.htmlFor = BakLn.bakLnCbCount;
      lab.innerHTML = txt;
      return addTdObj(lab);
    }
    // Create a checkbox as the first cell of the row
    var cbox = document.createElement("input");
    cbox.type = "checkbox";
    cbox.id = BakLn.bakLnCbCount;
    cbox.checked = getBoolItemOrDefault(`${appName}.bak-ln.${cbox.id}.checked`, false);
    addTdObj(cbox);
    //console.log(`${this.tagName}[${cbox.id}]:[${this.innerHTML}]`);
    this.innerHTML = '';
    this.style = "position: absolute;left:0px; top:0px; width:0px; height:0px;";
    // Add a second cell for the src of the backup
    var td_src = addTdLabel(this.getAttribute("src"));
    td_src.setAttribute("title", "UNIX[" + to_unix_path(this.getAttribute("src")) + "]");
    // Create a third cell as a decoration
    addTdLabel("&#x1F449;");
    // Add a fourth cell for the dst of the backup
    var td_dst = addTdLabel(this.getAttribute("dst"));
    td_dst.setAttribute("title", "WIN[" + to_win_path(this.getAttribute("dst")) + "]");
    // Add backup type as a dataset field
    tr.dataset.type = this.getAttribute("type");
    //console.log('TR:['+tr.innerHTML+']');
    cbox.addEventListener('change', function() {
      //console.log(`${appName}.bak-ln.${this.id}.checked: ${this.checked}`);
      localStorage.setItem(`${appName}.bak-ln.${this.id}.checked`, this.checked);
    });
    BakLn.bakLnCbCount++;
  }
  connectedCallback() {
    addEventListener("load", (event) => {
      if (HostName == "") {
        env.get("COMPUTERNAME").then((hn) => {
          HostName = hn;
          this.create();
        });
      } else {
        this.create();
      }
      //sortBakLn();
    });
  }
}
// Convert any character with code above 127 to its codepoint value compatible with the bash 'echo -e'
// Examples :
//   É ==> \u00c9
//   ☠ ==> \u2620
//   A ==> A
function to_echoe(s) {
  var res = "";
  for (i = 0; i < s.length; i++) {
    if (s.charCodeAt(i) > 127) {
      h = s.codePointAt(i).toString(16);
      r = "\\u" + "0000".substring(0, 4 - h.length) + h;
      res += r;
    } else {
      res += s.charAt(i);
    }
  }
  //console.log(res);
  return res;
}
async function run_backup(real_run = true) {
  bakLst = document.getElementById("backup-list");
  const rows = bakLst.getElementsByTagName("table").item(0).rows;
  shell_cmds = "";
  currPath = window.location.pathname;
  currPath = currPath.substring(1, currPath.lastIndexOf("/"));
  cyg_currPath = "/" + currPath.replace(/\\/g, "\/").replace(/:/, "");
  longer_rsync_cmd = "";
  longer_rsync_cmd_length = 0;
  for (i = 0; i < rows.length; i++) {
    cells = rows[i].cells;
    cbx = cells[0].firstChild;
    src = cells[1].firstChild;
    dst = cells[3].firstChild;
    typ = rows[i].dataset.type;
    if (cbx.checked === true) {
      if (shell_cmds !== "") shell_cmds += "; ";
      if (typ.startsWith("win-")) {
        typ = typ.slice(4);
        opt_param = "--no-links";
      } else {
        opt_param = "--chmod=755 --chown=nobody:nogroup";
      }
      if (typ == 'null' || typ == 'usr' || typ == 'user') {
        excl_file_title = cyg_currPath + "/user.exclude_list";
      } else {
        excl_file_title = cyg_currPath + "/famille.exclude_list";
      }
      rsync_cmd1 = "/usr/bin/rsync --progress -au " + opt_param + " --delete --delete-excluded --exclude-from=" + excl_file_title;
      //uxpth="/"+src.innerText.replace(/\\/g, "\/").replace(/:/, "");
      s_uxpth = to_unix_path(src.innerText);
      echo_cmd = `echo \\"\$(tput rmso)\$(tput smul)\$(tput setab 3 setaf 28)rsync parameters:\$(tput rmul)${rsync_cmd1.replace(/\/usr\/bin\/rsync /, "")} \$(tput sgr0)\\"; `;
      //    cmd+=`tput setab ${bgcol} setaf ${fgcol};`;
      //    cmd+=`echo '${msg}';`;
      //    cmd+="tput sgr0;";
      if (s_uxpth.endsWith("*")) {
        shTit = src.innerText; //.slice(0, -1)+'STAR';
        rsync_cmd = `for i in \"${s_uxpth}\"; do ${rsync_cmd1} \\"$i\\" '${dst.textContent}'`;
        echo_cmd += `for i in \"${s_uxpth}\"; do echo \\"rsync '\\"$i\\"' '${dst.textContent}';\\"; done`;
        //echo_cmd=`${rsync_cmd.replace(/\/usr\/bin\/rsync(.*)/g, 'echo \\"rsync $1')}\\"; done`;
        rsync_cmd += '; done';
      } else {
        shTit = src.innerText;
        if (s_uxpth.endsWith("/")) {
          rsync_cmd = `${rsync_cmd1} "${s_uxpth}" '${dst.textContent}'`; //-e "ssh -i $HOME/.ssh/id_rsa" ';
        } else {
          rsync_cmd = `${rsync_cmd1} "${s_uxpth}" "${dst.textContent}"`; //-e "ssh -i $HOME/.ssh/id_rsa" ';
        }
        rsync_cmd += ' -e \\"ssh -i $HOME/.ssh/id_rsa\\" ';
        echo_cmd += rsync_cmd.replace(/\/usr\/bin\/rsync(.*)/g, "echo 'rsync $1'");
      }
      shell_cmds += "echo; tput smso; echo 'Sauvegarde " + shTit + " dans " + dst.innerText + "'; ";
      shell_cmds += echo_cmd;
      if (real_run) shell_cmds += "; " + rsync_cmd;
      if (rsync_cmd.length > longer_rsync_cmd_length) {
        longer_rsync_cmd = rsync_cmd;
        longer_rsync_cmd_length = rsync_cmd.length;
      }
    }
  }
  //console.log("longer_rsync_cmd ("+longer_rsync_cmd_length+"): "+longer_rsync_cmd);
  term_width = 266;
  //console.log("run_backup: "+HostName);
  if (HostName === "FRTLS-2765") {
    msys_root = "C:\\Users\\denis.lalanne\\AppData\\Local\\Programs\\msys64"
  } else {
    msys_root = "C:\\UnixTools\\msys64"
  }
  //  -o ScrollbackLines=1000  == MaxScrollbackLines
  cmd_env = `${msys_root}\\usr\\bin\\mintty.exe -o ScrollbackLines=10000 -o Term=mintty -o Charset=UTF-8 -i app.ico -p 10,300 -s ${term_width},47 -t "Sauvegarde en cours" -h always -e /bin/bash --login -i -c `;
  //cmd=`${cmd_env} "${shell_cmds} tput smso smul; echo 'Sauvegarde terminée, appuyer sur <Entrée>'"`;
  cmd = cmd_env;
  if (real_run) {
    msg = "  B A C K U P   E N  C O U R S  ";
    fgcol = 255;
    bgcol = 124;
  } else {
    msg = "  D R Y   R U N  ";
    fgcol = 28;
    bgcol = 3;
  }
  spc = ' '.repeat(msg.length);
  col = Math.trunc((term_width - msg.length) / 2);
  //console.log(col);
  cmd += '"';
  // Shows 255 colors in background for TERM=mintty: for ((i=0; i < 255; i++)); do tput setab $i; printf " %03d " "$i"; tput sgr0; done
  // tput cup row col
  //    cmd+="export TERM=mintty;";
  cmd += `tput cup 0 ${col} bold;`;
  cmd += `tput setab ${bgcol} setaf ${fgcol};`;
  cmd += `echo '${msg}';`;
  cmd += "tput sgr0;";
  cmd += "echo;";
  if (shell_cmds !== '') cmd += ` ${shell_cmds}; `;
  msg = "  S A U V E G A R D E   T E R M I N É E ,   A P P U Y E R   S U R   < E N T R É E >  ";
  spc = ' '.repeat(msg.length);
  col = Math.trunc((term_width - msg.length) / 2);
  //    cmd+="echo; ";
  cmd += "tput u7; IFS=';' read -r -d R -a pos;row=$((${pos[0]:2} - 1)); "; //row=${pos[0]:2}; ";
  cmd += `tput bold setab 284; `;
  cmd += `tput cup $((\${row}+1)) ${col} bold; echo -e \\"${to_echoe(msg)}$(tput init rmso sgr0)\\";`;
  cmd += '"';
  backup_menu.style = "pointer-events:none;";
  console.log("RSYNC CMD:\n" + cmd);
  //  window.app.system(cmd);
  window.app.shell(cmd);
  // ===
  backup_menu.style = "pointer-events:auto;";
}
customElements.define("bak-ln", BakLn);
const delay = ms => new Promise(res => setTimeout(res, 400));
async function onloadcb() {
  HostName = await env.get("COMPUTERNAME");
  if (HostName == "FRTLS-2765") {
    bl = document.getElementById("backup-list");
    bakStr = '<bakLn src="C:\\Users\\denis.lalanne\\Documents\\CNES" dst="/u/CNES/CONNAISSANCES-GENERALES/Backup-DLA/MesDocs" type="win-usr"></bakLn>';
    bak = document.createElement("BakLn");
    bl.appendChild(bak);
  }
  sortBakLn();
}
window.addEventListener("load", async () => {
  document.getElementById("selall").addEventListener("click", async () => {
    const rows = document.getElementById("backup-list").getElementsByTagName("table").item(0).rows;
    for (i = 0; i < rows.length; i++) {
      cbx = rows[i].cells[0].firstChild;
      cbx.checked = true;
      localStorage.setItem(`${appName}.bak-ln.${i}.checked`, cbx.checked);
    }
  });
  document.getElementById("revsel").addEventListener("click", async () => {
    const rows = document.getElementById("backup-list").getElementsByTagName("table").item(0).rows;
    for (i = 0; i < rows.length; i++) {
      cbx = rows[i].cells[0].firstChild;
      cbx.checked = !cbx.checked;
      localStorage.setItem(`${appName}.bak-ln.${i}.checked`, cbx.checked);
    }
  });
  //console.log("End set sel buttons");
});