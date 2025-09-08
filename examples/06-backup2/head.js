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
  console.log(n);
}
async function load_backup_list() {
  var tbl = "<table>";
  var str = await fs.read("backup.list.csv");
  csv = str.split('\n');

  csv.forEach((row, ridx) => {
    if (row) {
      tbl += '<tr onclick="sel_row('+ridx+')">\n';
      cells = row.split(';');
      var docheck=false;
      cells.forEach((cell, cidx) => {
        tbl += '<td style="';
        if (cidx == 1) {
          if (cell == "yes") docheck=true;
          else docheck=false;
        }

        if (cidx == 3) tbl += 'border-right-style: none;';
        else if (cidx == 6) tbl += 'border-left-style: none;';
        else tbl += 'border-left-style: none; border-right-style: none;';

        if (cidx == 5) {
          tbl += 'text-align:end; padding-left:10px;';
        } else {
          if (ridx == 0 && cidx == 2) tbl += 'text-align:center;';
          else tbl += 'text-align:start;';
        }

        if (cidx == 3) {
          tbl += 'border-right-style: none;';
          tbl += '">';

          if (ridx > 0) {
            tbl += '<input type="checkbox"';
            if (docheck) tbl += "checked";
            tbl += '/>';
          }
        } else tbl += '">';

        if (cidx == 6) tbl += ':';

        if (cidx == 5 && !cell) cell = "localhost";
        tbl += cell;

        if (cidx == 4) {
          tbl += '<td style="border-left-style: none; border-right-style: none;">';
          if (ridx > 0) tbl += '&#x1F449';
          tbl += '</td>';
        }

        tbl += "</td>\n";
      });
      tbl += "</tr>";
    }
  });

  tbl += "</table>";
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
