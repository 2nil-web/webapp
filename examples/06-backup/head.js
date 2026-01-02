function clean_exit() {
  save_backup_list();
  //  if (confirm("Exit"))
  app.exit();
}

function esc_exit() {
  if (event.keyCode === 27) clean_exit();
}

var bak_fn = "backup.list.csv";
var HostName = "";

/*  CSV   : Type => 0;ToDo => 1;Source host => 2;Source directory => 3;        Destination [user@] => 4;[host:] => 5;directory => 6
    TABLE : Type => 0;ToDo => 1;Source host => 2;Source directory => 3;👉 => 4;Destination [user@][host:]directory => 5
                                                                       ======== */
function save_backup_list() {
  var bak_text = "Type;ToDo;Source host;Source directory;Destination [user@];[host:];directory\n";
  for (var r = 1; r < backup_list.rows.length; r++) {
    for (var c = 0; c < backup_list.rows[r].cells.length; c++) {
      cell = backup_list.rows[r].cells[c];

      if (c == 1) {
        var cb = cell.children[0];
        if (cb.checked) bak_text += "yes";
        else bak_text += "no";
        bak_text += ';';
      } else if (c > 4) {
        var uhd = cell.innerText.split('@'),
          hd = "";
        if (uhd.length >= 2) {
          bak_text += uhd[0];
          hd = uhd[1];
        } else {
          hd = uhd[0];
        }

        bak_text += ';';

        var d = hd.split(':');
        if (d.length >= 2) {
          bak_text += d[0] + ';' + d[1];
        } else {
          bak_text += ';' + d[0];
        }

      } else if (c != 4) bak_text += cell.innerText + ';';
    }
    bak_text += '\n';
  }
  //console.log(bak_text);
  fs.write(bak_fn, bak_text);
}

function select(all = true) {
  for (var r = 1; r < backup_list.rows.length; r++) {
    row = backup_list.rows[r];
    //console.log(`${row.cells[2].innerText} == ${HostName}`);

    // Do not modify lines of another host
    if (row.cells[2].innerText == HostName) {
      var cb = row.cells[1].children[0];
      if (all) cb.checked = true;
      else cb.checked = !cb.checked;
    }
  }
}

function sel_row(n) {
  var e = window.event;
  if (document.elementFromPoint(e.clientX, e.clientY).tagName == "TD") {
    var cb = backup_list.rows[n].cells[1].children[0];
    cb.checked = !cb.checked;
  }
}
async function load_backup_list() {
  HostName = await env.get("COMPUTERNAME");

  var str = await fs.read(bak_fn);
  csv = str.split('\n');
  var tbl = "<table>";

  csv.forEach((row, ridx) => {
    if (row) {
      cells = row.split(';');
      var uhd = "";
      var notMyHost = false;

      var rws = "";
      cells.forEach((cell, cidx) => {
        if (cidx < 4) {
          if (cidx == 1) {
            if (ridx == 0) {
              rws += "<td></td>";
            } else {
              cell = cell.toLowerCase();
              rws += '<td><input type="checkbox"';
              if (cell == "yes" || cell == "true" || cell == "ok" || cell == "1") rws += " checked";
              rws += '/></td>';
            }
          } else {
            if (cidx == 2 && ridx > 0 && cell != HostName) {
              notMyHost = true;
            }
            rws += '<td>' + cell + "</td>";
          }
        } else {
          uhd += cell;

          if (cidx == 4) {
            rws += '<td>';
            if (ridx > 0) {
              rws += '&#x1F449';
              if (cell) uhd += '@';
            }

            rws += '</td>';
          } else {
            if (ridx > 0 && cidx == 5 && cell) uhd += ':';
            else if (cidx == 6) {
              rws += "<td>" + uhd + "</td>";
              uhd += cell;
            }
          }
        }
      });

      tbl += '<tr onclick="sel_row(' + ridx + ')"';

      // Deactivate lines of other host
      if (notMyHost) {
        //console.log(rws);
        tbl += 'style="display: none;"';
        rws = rws.replace(/<input type="checkbox" checked\/>/, '<input type="checkbox"/>');
      }

      //console.log(rws);
      tbl += '>' + rws + "</tr>";
    }
  });

  tbl += "</table>";
  backup_list.innerHTML = tbl;
  /*
    console.log(tbl);
    pretty_tbl=tbl.replace(/(<table>)/, "$1\n").replace(/(<td>)/g, "\n    $1").replace(/(<tr)/g, "  $1").replace(/(<\/tr>)/g, "\n  $1\n");
    console.log(pretty_tbl);
  */
  app.on_close("clean_exit()");
}

async function eventFunc(obj, evtStr, func) {
  obj.addEventListener(evtStr, async () => {
    // Disable esc_exit
    document.removeEventListener("keyup", esc_exit);
    await func();
    // Force the mouse pointer to its previous position
    document.body.requestPointerLock();
    document.exitPointerLock();
    // Re-enable esc_exit after a little while
    await new Promise(r => setTimeout(r, 600));
    document.addEventListener("keyup", esc_exit);
  });
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

// TABLE : Type => 0;ToDo => 1;Source host => 2;Source directory => 3;👉 => 4;Destination [user@][host:]directory => 5
async function run_backup(real_run = true) {
  shell_cmds = "";
  currPath = window.location.pathname;
  currPath = currPath.substring(1, currPath.lastIndexOf("/"));
  cyg_currPath = "/" + currPath.replace(/\\/g, "\/").replace(/:/, "");
  longer_rsync_cmd = "";
  longer_rsync_cmd_length = 0;

  for (var i = 1; i < backup_list.rows.length; i++) {
    cells = backup_list.rows[i].cells;
    typ = cells[0].innerText;
    cbx = cells[1].firstChild;
    src = cells[3];
    dst = cells[5];

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
        //rsync_cmd += ' -e \\"ssh -i $HOME/.ssh/id_rsa\\" ';
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
  run.style = "pointer-events:none;";
  console.log("RSYNC CMD:\n" + cmd);
  //  window.app.system(cmd);
  window.app.shell(cmd);
  // ===
  run.style = "pointer-events:auto;";
}


if (typeof app.sysname !== "undefined") {
  app.set_icon("app.ico");
  app.set_title(document.title);

  load_backup_list();

  async function do_load() {
    document.addEventListener("keyup", esc_exit);

    eventFunc(sel_all, "click", async (e) => {
      select();
    });

    eventFunc(sel_rev, "click", async (e) => {
      select(false);
    });

    eventFunc(run, "click", async (e) => {
      run_backup();
    });

    eventFunc(run_dry, "click", async (e) => {
      run_backup(false);
    });

    quit.addEventListener("click", (e) => {
      clean_exit();
    });

    eventFunc(about, "click", async () => {
      await gui.msgbox("Backup tool V2, based on " + app.info);
    });
  }

  window.addEventListener("load", do_load);
}