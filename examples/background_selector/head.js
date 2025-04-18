var output_to_console = true;

function cls() {
  if (output_to_console) console.clear();
  else {
    body.innerHTML = "";
    body.scrollTop = body.scrollHeight;
  }
}

var not_ln = "";

function print(s = "", scrollToBottom = true) {
  if (output_to_console) not_ln += s;
  else {
    body.innerHTML += s;
    if (scrollToBottom) body.scrollTop = body.scrollHeight;
  }
}

function println(s = "", scrollToBottom = true) {
  if (output_to_console) {
    if (not_ln.length > 0) {
      s += not_ln;
      not_ln = "";
    }
    console.log(s);
  } else {
    body.innerHTML += s + "<br/>";
    if (scrollToBottom) body.scrollTop = body.scrollHeight;
  }
}


function compute_gcd(a, b) {
  return b ? compute_gcd(b, a % b) : a;
}
// Reduce a fraction by finding the Greatest Common Divisor and dividing by it.
function reduce(numerator, denominator) {
  var gcd = compute_gcd(numerator, denominator);
  return [numerator / gcd, denominator / gcd];
}

function create_img(name, data, il, it, ir, ib) {
  var img = document.createElement('img');
  //  get file in base64 data
  img.setAttribute('src', data);
  img.style.left = il + "px";
  img.style.top = it + "px";
  img.style.width = (ir - il) + "px";
  img.style.height = (ib - it) + "px";
  img.style.zIndex = 2;
  img.style.position = "absolute";
  console.log(img.style);
  body.appendChild(img);
}


async function get_virtual_resolution(trace = true) {
  var wpi = await win.wallpapers_info();
  var n_monitors = Object.keys(wpi).length;

  if (n_monitors > 0) {
    // Rect[] is left, top, right, bottom
    var out_x_or_y = 100000;
    var vleft = out_x_or_y,
      vtop = out_x_or_y,
      vright = -out_x_or_y,
      vbottom = -out_x_or_y,
      vwidth, vheight;

    for (k in wpi) {
      //console.log(`Processing ${k}`);
      var cwi = wpi[k];
      var rc = cwi["rect"];
      // Does not consider wallpaper if its monitor has a rect without coordinate
      if (rc.length !== 4) continue;

      // Try to compute virtual screen coordinate if more than 1 monitor with wallpaper
      if (n_monitors > 1) {
        if (rc[0] < vleft) vleft = rc[0];
        if (rc[1] < vtop) vtop = rc[1];
        if (rc[2] > vright) vright = rc[2];
        if (rc[3] > vbottom) vbottom = rc[3];
      }

      vwidth = vright - vleft;
      vheight = vbottom - vtop;
    }

    var html = document.documentElement;
    var bodyWidth = Math.max(body.scrollWidth, body.offsetWidth, html.clientWidth, html.scrollWidth, html.offsetWidth);
    var bodyHeight = Math.max(body.scrollHeight, body.offsetHeight, html.clientHeight, html.scrollHeight, html.offsetHeight);
    var kW = bodyWidth / vwidth;
    var kH = bodyHeight / vheight;

    println(`My page as dimensions ${bodyWidth}, ${bodyHeight})`);
    println(`Virtual screen of dimension(${vwidth}, ${vheight})`);
    println(`Coeff reduction dimension(${kW}, ${kH})`);
    for (k in wpi) {
      var cwi = wpi[k];
      var rc = cwi["rect"];
      // Does not consider wallpaper if its monitor has a rect without coordinate
      if (rc.length !== 4) continue;

      var nRc = [bodyWidth * rc[0] / vwidth, bodyHeight * rc[1] / vheight, bodyWidth * rc[2] / vwidth, bodyHeight * rc[3] / vheight];
      println(`Display ${cwi["file"]} with orignal coord (${rc}) in new coord (${nRc})`);
      var b64_img = "data:image/jpeg;base64," + (await fs.read_to_base64(cwi["file"]));
      //console.log(b64_img);
      create_img("bg_" + k, b64_img, nRc[0], nRc[1], nRc[2], nRc[3]);
    }
  }

  //return [];
}

async function next_wallpaper_infos() {
  var owi = await win.wallpapers_info();
  console.log(owi);
  var nw = await win.next_wallpaper();

  if (nw) {
    var no_wp_change = true;

    var to = setTimeout(() => {
      console.log("Time out while trying to change wallpaper");
      no_wp_change = false;
    }, 6000);

    while (no_wp_change) {
      var nwi = await win.wallpapers_info();
      i = 0;
      //console.log("loop on wallpaper files per monitor");
      for (k in owi) {
        if (owi[k]["file"] !== nwi[k]["file"]) {
          clearTimeout(to);
          println(`On monitor number ${i+1}, coordinates (${nwi[k]["rect"]}), wallpaper has change from file ${owi[k]["file"]} to file ${nwi[k]["file"]}`)
          no_wp_change = false;

        }
        i++;
      }
      await sleep(0.1);
    }
  } else {
    println("Unable to advance to next wallpaper");
  }
}

if (typeof app.sysname !== "undefined") {
  app.set_icon("app.ico");
  app.set_title(document.title);
  window.addEventListener("load", do_load);

  async function do_load() {
    // Prevent the app to load multiple times
    const bc = new BroadcastChannel(document.title);
    bc.postMessage(document.title);
    bc.onmessage = (event) => {
      if (event.data === document.title) {
        (async () => {
          await app.bring_to_top();
          bc.postMessage("already_loaded " + app.window_id);
        })();
      } else if (event.data.startsWith("already_loaded")) {
        (async () => {
          var win_id = event.data.substr(15);
          //console.log("2éme: ["+win_id+']');
          //await app.echo("from js:"+win_id);
          await app.bring_to_top(win_id);
          app.exit();
        })();
      }
    }
    //console.log(app.window_id);

    body = document.getElementsByTagName('body')[0];

    function clean_exit() {
      bc.close();
      app.exit();
    }

    function exit_on_esc() {
      if (event.keyCode === 27) clean_exit();
    }
    document.addEventListener("keyup", exit_on_esc);

    if (app.sysname === "Windows") {
      app.show();
      //println("We're OK");
      get_virtual_resolution();
      //console.log(await win.monitors_info());
      //console.log(await win.devices_info());
    } else(async () => {
      await app.center();
      await app.set_size(300, 120, 1);
      await app.set_size(300, 120, 2);
      println("<center>Sorry, this app only run under Windows<br /><br /><button style='width:6em' onclick='app.exit()'>  OK  </button></center>");
      app.show();
    })();
  }
}