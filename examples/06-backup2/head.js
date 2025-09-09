
/*    CSV   : Type => 0;ToDo => 1;Source host => 2;Source directory => 3;        Destination [user@] => 4;[host:] => 5;directory => 6
      TABLE : Type => 0;ToDo => 1;Source host => 2;Source directory => 3;👉 => 4;Destination [user@][host:]directory => 5
                                                                         ========
*/
var bak_fn="backup.list.csv";

function save_backup_list() {
  var bak_text="Type;ToDo;Source host;Source directory;Destination [user@];[host:];directory\n";
  for (var r=1; r < backup_list.rows.length; r++) {
    for (var c=0; c < backup_list.rows[r].cells.length; c++) {
      cell=backup_list.rows[r].cells[c];

      if (c == 1) {
        var cb=cell.children[0];
        if (cb.checked) bak_text += "yes";
        else bak_text += "no";
        bak_text+=';';
      } else if (c > 4) {
        var uhd=cell.innerText.split('@'), hd="";
        if (uhd.length >= 2) {
          bak_text+=uhd[0];
          hd=uhd[1];
        } else {
          hd=uhd[0];
        }

        bak_text+=';';

        var d=hd.split(':');
        if (d.length >= 2) {
          bak_text+=d[0]+';'+d[1];
        } else {
          bak_text+=';'+d[0];
        }

      } else if (c != 4) bak_text += cell.innerText+';';
    }
    bak_text += '\n';
  }
  console.log(bak_text);
  fs.write(bak_fn, bak_text);
}

function clean_exit() {
  save_backup_list();
//  if (confirm("Exit"))
    app.exit();
}

function select(all=true) {
  for (var r=1; r < backup_list.rows.length; r++) {
    cell=backup_list.rows[r].cells[1];
    var cb=cell.children[0];
    if (all) cb.checked=true;
    else cb.checked=!cb.checked;
  }
}

function esc_exit() {
  if (event.keyCode === 27) clean_exit();
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

async function renable_esc_exit(evt) {
  // Re-enable esc_on_exit after a little while
  await new Promise(r => setTimeout(r, 100));
  document.addEventListener("keyup", esc_exit);
}

function sel_row (n) {
  var e = window.event;
  if (document.elementFromPoint(e.clientX, e.clientY).tagName == "TD") {
    var cb=backup_list.rows[n].cells[1].children[0];
    cb.checked=!cb.checked;
  }
}
async function load_backup_list() {
  var tbl = "<table>";
  var str = await fs.read(bak_fn);
  csv = str.split('\n');
  csv.forEach((row, ridx) => {
    if (row) {
      tbl += '<tr onclick="sel_row('+ridx+')">';
      cells = row.split(';');
      var uhd="";

      cells.forEach((cell, cidx) => {
        if (cidx < 4) {
          if (cidx == 1) {
            if (ridx == 0) {
              tbl += "<td></td>";
            } else {
              cell=cell.toLowerCase();
              tbl += '<td><input type="checkbox"';
              if (cell == "yes" || cell == "true" || cell == "ok" || cell == "1") tbl += "checked";
              tbl += '/></td>';
            }
          } else
            tbl += '<td>'+cell+"</td>";
        } else {
          uhd+=cell;

          if (cidx == 4) {
             tbl += '<td>';
            if (ridx > 0) {
              tbl += '&#x1F449';
              if (cell) uhd+='@';
            }

            tbl += '</td>';
          } else {
            if (ridx > 0 && cidx == 5 && cell) uhd+=':';
            else if (cidx == 6) {
              tbl += "<td>"+uhd+"</td>";
              uhd+=cell;
            }
          }
        }
      });
      tbl += "</tr>";
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
      await gui.msgbox("run");
    });

    eventFunc(run_dry, "click", async (e) => {
      await gui.msgbox("run_dry");
    });

    quit.addEventListener("click", (e) => {
      clean_exit();
    });

    eventFunc(about, "click", async () => {
      await gui.msgbox("Backup tool V2, based on " + app.info);
    });
    /*
        about.addEventListener("click", async (e) => {
          disable_esc_exit(e);
          await gui.msgbox("Backup tool V2, based on "+app.info);
          renable_esc_exit(e);
        console.log(about.style);
        });*/
  }

  window.addEventListener("load", do_load);
}
