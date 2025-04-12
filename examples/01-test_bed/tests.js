//for (const key of Object.keys(localStorage)) { console.log("onload "+key, localStorage.getItem(key)); }
cls();
async function blink(id, nsec = 2) {
  var el = document.getElementById(id);
  var osty = el.style;
  //console.log(osty);
  for (var i = 0; i < 4; i++) {
    el.style = "font-size:14px;font-weight:bold;color:red;";
    await sleep(1 * nsec / 8);
    el.style = "font-size:14px;font-weight:bold;font-style:italic;color:green;";
    await sleep(1 * nsec / 8);
  }
  el.style = osty;
}
blink_id = -1;
async function start_blink(id) {
  var el = document.getElementById(id);
  var osty = el.style;
  blink_id = id;
  //console.log(osty);
  while (blink_id !== -1) {
    el.style = "font-size:14px;font-weight:bold;color:red;";
    await sleep(0.2);
    el.style = "font-size:14px;font-weight:bold;font-style:italic;color:green;";
    await sleep(0.2);
  }
  el.style = "font-size:14px; " + osty;
}

function stop_blink() {
  blink_id = -1;
}

function group_test(me, nb = 7) {
  var idx = Number(me.id.match(/\d+/)[0]);
  if (me.indeterminate === true || typeof me.checked === "undefined") return;
  //console.log(`grp_tst: ${me.id} (${me.id.match(/\d+/)[0]}), idx: ${idx}`);
  for (var i = 0; i < nb; i++) {
    cb = document.getElementById(`id_cb${idx+i+1}`);
    if (cb) cb.checked = me.checked;
  }
}
async function start_test(me, nsec = 2) {
  var idx = tests.indexOf(me);
  //console.log(`Running test '${me[0]}', idx: ${idx}`);
  await blink("for_id_cb" + idx, nsec);
  return idx;
}
async function toto2lab(me, idx) {
  document.getElementById("for_id_cb" + idx).innerHTML = `${me[0]} ==> TOTO=${await env.get("TOTO")}`;
}
async function fslab(me, res) {
  var idx = await start_test(me);
  var lab = document.getElementById("for_id_cb" + idx)
  lab.style.fontSize = "14px";
  lab.innerHTML = `${me[0]}${res}`;
}
async function path2lab(me, idx) {
  var re;
  //console.log('['+(await env.get_path())+']');
  if (is_windows) re = new RegExp(';', "g");
  else re = new RegExp(':', "g");
  document.getElementById("for_id_cb" + idx).innerHTML = `${me[0]} :\n${(await env.get_path()).replace(re, "\n")}`;
}
async function resetlab(idx) {
  var lab = document.getElementById("for_id_cb" + idx);
  lab.innerHTML = lab.innerHTML.substring(0, lab.innerHTML.indexOf(':'));
  //  console.log(lab.innerHTML);
}
async function dlglab(me, cmd) {
  var idx = tests.indexOf(me);
  var lab = document.getElementById("for_id_cb" + idx);
  start_blink(lab.id);
  document.removeEventListener("keyup", exit_on_esc);
  console.log(cmd);
  var res = await eval(cmd);
  savHtml = lab.innerHTML;
  //lab.style.fontSize="14px";
  if (res.length === 0) res = "nothing";
  else res = ":<br/>" + res;
  lab.innerHTML = `${me[0]} You have chosen <span style="font-size:14px; font-style:italic; ">${res}</span>`;
  document.addEventListener("keyup", exit_on_esc);
  await sleep(0.8);
  stop_blink();
  sleep(0.2);
}
// Array of arrays, in each subarray:
//  [0] is the description of the test
//  [1] if exists, is the function running the test OR ...
//  [2] if exists, this is a test group title
var tests = [
  ["The app functions", function() {
    group_test(this, 10);
  }, true, ],
  ["The window upper left icon and title will change during 3 seconds",
    async function() {
      var idx = await start_test(this);
      old_title = await app.set_title(" <====== CHANGED ICON AND TITLE ! !");
      old_ico = await app.set_icon("");
      await sleep(3);
      app.set_title(old_title);
      app.set_icon(old_ico);
    },
  ],
  ["The windows will go to the tray bar for 2 seconds and then re-appears",
    async function() {
      var idx = await start_test(this);
      app.minimize();
      await sleep(2);
      app.restore();
    },
  ],
  ["The windows will be maximized for 2 seconds and then be restored",
    async function() {
      var idx = await start_test(this);
      app.maximize();
      await sleep(2);
      app.restore();
    },
  ],
  ["The windows will totally disappears for 2 seconds and then re-appears",
    async function() {
      var idx = await start_test(this);
      app.hide();
      await sleep(2);
      app.show();
    },
  ],
  ["The window position will change and be restored to its initial state after 2 seconds",
    async function() {
      var idx = await start_test(this);
      pos = await app.set_pos(10, 10);
      await sleep(2);
      app.set_pos(pos.x, pos.y);
    },
  ],
  ["The window size will change and be restored to its initial state after 2 seconds",
    async function() {
      var idx = await start_test(this);
      old_sz = await app.set_size(800, 420, 0);
      console.log(old_sz.w + ", " + old_sz.h);
      await sleep(2);
      app.set_size(old_sz.w, old_sz.h, 0);
    },
  ],
  ["The whole window geometry will change and be restored to its initial state after 2 seconds",
    async function() {
      var idx = await start_test(this);
      old_geo = await app.set_geometry(10, 10, 900, 420);
      await sleep(2);
      console.log(old_geo.x, old_geo.y, old_geo.w, old_geo.h);
      app.set_geometry(old_geo.x, old_geo.y, old_geo.w, old_geo.h);
    },
  ],
  [],
  ["A file explorer appears\n<i>And you may notice some movements on it, while playing fs functions tests</i>",
    async function() {
      start_test(this);
      app.shell(`explorer.exe ${await fs.current_path()}`);
    },
  ],
  ["The env functions", function() {
    group_test(this, 8);
  }, true, ],
  ["Setting the environment variable 'TOTO' to the value '12'",
    async function() {
      var idx = await start_test(this);
      await env.set("TOTO", 12);
      toto2lab(this, idx);
    },
  ],
  ["Inserting the string 'before' to 'TOTO'",
    async function() {
      var idx = await start_test(this);
      await env.ins("TOTO", "before ");
      toto2lab(this, idx);
    },
  ],
  ["Adding the string 'after' to 'TOTO'",
    async function() {
      var idx = await start_test(this);
      await env.add("TOTO", " after");
      toto2lab(this, idx);
    },
  ],
  ["Deleting the 'TOTO' variable",
    async function() {
      var idx = await start_test(this);
      await env.del("TOTO");
      toto2lab(this, idx);
    },
  ],
  [],
  ["Inserting a new path to the standard PATH variable",
    async function() {
      var idx = await start_test(this);
      actualPath = await env.get_path();
      await env.ins_path("/path/before");
      path2lab(this, idx);
    },
  ],
  ["Adding a new path to the standard PATH variable",
    async function() {
      var idx = await start_test(this);
      await env.add_path("/path/after");
      resetlab(idx - 1);
      path2lab(this, idx);
    },
  ],
  ["Resetting PATH to its initial value",
    async function() {
      var idx = await start_test(this);
      await env.set_path(actualPath);
      await sleep(3);
      resetlab(idx - 1);
      path2lab(this, idx);
    },
  ],
  ["The fs functions", function() {
    group_test(this, 19);
  }, true, ],
  ["Creating directory 'dir基极及' (whether it exists or not)", async function() {
    await fslab(this, ` ==> ${await fs.mkdir("dir基极及")}`);
  }, ],
  ["Displaying informations for the directory 'dir基极及'", async function() {
    await fslab(this, ` ==> ${await fs.stat("dir基极及")}`);
  }, ],
  ["Changing current working directory to 'dir基极及' (if it exists)", async function() {
    await fslab(this, ` ==> ${await fs.current_path("dir基极及")}`);
  }, ],
  ["Displaying current path", async function() {
    //await fslab(this, ` ==> ${res=(await fs.current_path())?res:fs.last_error}`);
    await fslab(this, ` ==> ${fs.cwd}`);
  }, ],
  ["Removing file 'file基极及.txt' (whether it exists or not)", async function() {
    await fslab(this, ` ==> ${await fs.remove("file基极及.txt")}`);
  }, ],
  ["Testing the existence of the file 'file基极及.txt' (should return 'false', if previous test has been run)", async function() {
    await fslab(this, ` ==> ${await fs.exists("file基极及.txt")}`);
  }, ],
  ["Creating the file 'toto🤖🔥.txt' with the content 'toto string'", async function() {
    await fslab(this, ` ==> ${await fs.write("toto🤖🔥.txt", "toto string")}`);
  }, ],
  ["Appending the string ' 💥🧸🤖🔥=安暗按' as a second line to the file 'dir基极及/toto.txt'", async function() {
    await fslab(this, ` ==> ${await fs.append("toto🤖🔥.txt", "\n💥🧸🤖🔥=安暗按")}`);
  }, ],
  ["Testing the existence of the file 'toto🤖🔥.txt'", async function() {
    await fslab(this, ` ==> ${await fs.exists("toto🤖🔥.txt")}`);
  }, ],
  ["Displaying the full path of the file 'toto🤖🔥.txt'", async function() {
    await fslab(this, ` ==> ${await fs.absolute("toto🤖🔥.txt")}`);
  }, ],
  ["Displaying informations about the file 'toto🤖🔥.txt'", async function() {
    await fslab(this, ` ==> ${await fs.stat("toto🤖🔥.txt")}`);
  }, ],
  ["Reading the content of the file 'toto🤖🔥.txt'", async function() {
    await fslab(this, `:\n${await fs.read("toto🤖🔥.txt")}`);
  }, ],
  ["Listing of the current path", async function() {
    await fslab(this, `:\n${await fs.ls()}`);
  }, ],
  ["Changing current working directory to '..'", async function() {
    await fslab(this, `:\n${await fs.current_path("..", true)}`);
  }, ],
  ["Detailled listing of the 'dir基极及'", async function() {
    await fslab(this, `:\n${(await fs.ls("dir基极及", true, "html"))}`);
  }, ],
  ["Copying the directory 'dir基极及' to 'tutu'", async function() {
    await fslab(this, ` ==> ${await fs.copy("dir基极及", "tutu")}`);
  }, ],
  ["Removing the not empty directory 'dir基极及'", async function() {
    await fslab(this, ` ==> ${await fs.remove_all("dir基极及")}`);
  }, ],
  ["Removing the file 'tutu/toto🤖🔥.txt'", async function() {
    await fslab(this, ` ==> ${await fs.remove("tutu/toto🤖🔥.txt")}`);
  }, ],
  ["Removing the empty directory 'tutu'", async function() {
    await fslab(this, ` ==> ${await fs.remove("tutu")}`);
  }, ],
];
Array.prototype.push.apply(tests, [
  ["GUI functions", function() {
    group_test(this, 3);
  }, true, ],
  ["File dialog allowing to select filtered files in specific directory.",
    async function() {
      await dlglab(this, 'gui.opendlg("*.js,*.html;Javascript or html", "..", "Load a javascript or html file");');
    },
  ],
  ["A file saving dialog appears.",
    async function() {
      await dlglab(this, 'gui.savedlg("*.js,*.html;Javascript or html", "..", "Save a javascript or html file")');
    },
  ],
  ["A folder selection dialog appears.",
    async function() {
      await dlglab(this, 'gui.folderdlg("", "/", "Select a folder")');
    },
  ],
]);

function creatSpan(str) {
  var sp = document.createElement('span');
  sp.innerHTML = str;
  output.appendChild(sp);
}

function StoreCbState(id) {
  var el = document.getElementById(id);
  var cb_val = el.checked ? "true" : "false";
  //console.log(`Storing ${id}=${cb_val}`);
  localStorage.setItem(id, cb_val);
}

function RetrieveCbState(el) {
  var valcb = localStorage.getItem(el.id);
  var cb_bool = true;
  if (valcb === null || valcb === "" || valcb !== "true") {
    cb_bool = false;
    localStorage.setItem(el.id, valcb);
  } else {
    //console.log(`${el.id} : ${valcb}`);
  }
  el.checked = cb_bool;
}

function changecb() {
  //console.log(`cb ${this.id}`);
  StoreCbState(this.id);
}

function creatCkBox(str, id) {
  var cb = document.createElement('input');
  cb.type = 'checkbox';
  cb.id = id;
  RetrieveCbState(cb);
  cb.onchange = changecb;
  var lab = document.createElement("label");
  lab.style = "font-size:14px;";
  lab.setAttribute("for", id);
  lab.setAttribute("id", "for_" + id);
  lab.innerHTML = str;
  //lab.appendChild(document.createTextNode(str));
  output.appendChild(cb);
  output.appendChild(lab);
  return cb;
}
async function run_tests() {
  for (var i = 0; i < tests.length; i++) {
    test = tests[i];
    if (test.length > 1 && test.length < 3 && document.getElementById("id_cb" + i).checked) {
      var isAsync = test[1][Symbol.toStringTag] === 'AsyncFunction';
      if (isAsync) {
        //console.log(`Test ${i} is a Asynchronous.`);
        //await blink("for_id_cb" + i);
        await test[1]();
        await sleep(1);
      } else {
        console.log(`Test ${i} is a synchronous`);
        blink("for_id_cb" + i);
        test[1]();
      }
    }
  }
}

function sel_all(stat) {
  for (i = 0; i < tests.length; i++) {
    cb = document.getElementById("id_cb" + i);
    if (cb) {
      if (stat < 0) {
        cb.checked = !cb.checked;
      } else cb.checked = stat;
      StoreCbState("id_cb" + i);
    }
  }
}

function creatBut(str, func) {
  var but = document.createElement('input');
  but.type = 'button';
  but.value = str;
  but.title = str;
  but.onclick = func;
  but.style.marginTop = "2px";
  but.style.marginLeft = "4px";
  but.style.marginBottom = "4px";
  output.appendChild(but);
}
creatBut("Run selected", run_tests);
creatBut("Select all", () => {
  sel_all(1)
});
creatBut("Unselect all", () => {
  sel_all(0)
});
creatBut("Reverse selection", () => {
  sel_all(-1)
});
output.appendChild(document.createElement("br"));
for (var i = 0; i < tests.length; i++) {
  test = tests[i];
  if (test.length > 0) {
    if (test.length > 1) {
      if (test.length > 2) output.appendChild(document.createElement("br"));
      var el = creatCkBox(test[0], "id_cb" + i);
      if (test.length > 2) {
        var lab = document.getElementById("for_id_cb" + i);
        lab.style = "font-size:14px;text-decoration: underline;";
        el.title = lab.title = "Click to temporarily toogle the whole functions group";
        el.indeterminate = true;
        el.addEventListener('change', test[1]);
      }
    } else {
      // Comment
      creatSpan(test[0]);
    }
  }
  output.appendChild(document.createElement("br"));
}