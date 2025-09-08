function esc_exit() {
  if (event.keyCode === 27) app.exit();
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
  cb=backup_list.rows[n].cells[3].children[0];
  cb.checked=!cb.checked;
}
async function load_backup_list() {
  var tbl = "<table>\n";
  var str = await fs.read("backup.list.csv");
  csv = str.split('\n');
  csv.forEach((row, ridx) => {
    if (row) {
      tbl += '  <tr onclick="sel_row('+ridx+')">\n';
      cells = row.split(';');
      var docheck=false;
      var uhd="";

      cells.forEach((cell, cidx) => {
        if (cidx == 1) {
          cell=cell.toLowerCase();
          if (cell == "yes" || cell == "true" || cell == "ok" || cell == "1") docheck=true;
          else docheck=false;
        }

        if (cidx < 3) {
          tbl += '    <td>'+cell+"</td>\n";
        } else if (cidx == 3) {
          tbl += "    <td>";

          if (ridx > 0) {
            tbl += '<input type="checkbox"';
            if (docheck) tbl += "checked";
//            tbl += ' disabled';
            tbl += '/>';
          }

          tbl +=cell+"</td>\n"
        } else {
          uhd+=cell;

          if (cidx == 4) {
             tbl += '    <td>';
            if (ridx > 0) {
              tbl += '&#x1F449';
              if (cell) uhd+='@';
            }

            tbl += '</td>\n';
          } else {
            if (cidx == 5 && cell) uhd+=':';
            else if (cidx == 6) {
              tbl += "    <td>"+uhd+"</td>\n";
              uhd+=cell;
            }
          }
        }
      });
      //console.log('uhd['+uhd+']');
      tbl += "  </tr>\n";
    }
  });

  tbl += "\n</table>";
  backup_list.innerHTML = tbl;
  console.log(tbl);
}

if (typeof app.sysname !== "undefined") {
  app.set_icon("app.ico");
  app.set_title(document.title);

  load_backup_list();

  async function do_load() {
    document.addEventListener("keyup", esc_exit);

    eventFunc(sel_all, "click", async (e) => {
      await gui.msgbox("sel_all");
    });

    eventFunc(sel_rev, "click", async (e) => {
      await gui.msgbox("sel_rev");
    });

    eventFunc(run, "click", async (e) => {
      await gui.msgbox("run");
    });

    eventFunc(run_dry, "click", async (e) => {
      await gui.msgbox("run_dry");
    });

    quit.addEventListener("click", (e) => {
      app.exit();
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
