var img_divs = null;

async function sleep(nsec) {
  await new Promise(r => setTimeout(r, nsec * 1000));
}

async function next_wallpaper_infos() {
  var owi = await win.wallpapers_info();
  //console.log(owi);
  var nw = await win.next_wallpaper();

  if (nw) {
    var no_wp_change = true;

    var to = setTimeout(() => {
      //console.log("Time out while trying to change wallpaper");
      no_wp_change = false;
    }, 6000);

    while (no_wp_change) {
      var nwi = await win.wallpapers_info();
      i = 0;
      //console.log("loop on wallpaper files per monitor");
      for (k in owi) {
        if (owi[k]["file"] !== nwi[k]["file"]) {
          clearTimeout(to);
          //console.log(`On monitor number ${i+1}, coordinates (${nwi[k]["rect"]}), wallpaper has change from file ${owi[k]["file"]} to file ${nwi[k]["file"]}`)
          no_wp_change = false;

        }
        i++;
      }
      await sleep(0.1);
    }
  } else {
    //console.log("Unable to advance to next wallpaper");
  }
}

var cnf_svg = '<g transform="matrix(.26996 0 0 .27667 -67.369 2.0244)"><path d="m293.4-3.51c-2.74 0-5.41 0.28-8 0.81v6.18c-3.47 0.83-6.72 2.18-9.68 4l-4.38-4.375c-2.24 1.48-4.29 3.22-6.19 5.125-1.89 1.89-3.64 3.94-5.12 6.18l4.37 4.375c-1.81 2.967-3.16 6.212-4 9.687h-6.18c-0.53 2.585-0.82 5.261-0.82 8s0.29 5.415 0.82 8h6.18c0.84 3.475 2.19 6.72 4 9.688l-4.37 4.375c1.48 2.242 3.23 4.292 5.12 6.187 1.9 1.895 3.95 3.639 6.19 5.125l4.38-4.375c2.96 1.816 6.21 3.161 9.68 4v6.188c2.59 0.525 5.26 0.812 8 0.812s5.42-0.287 8-0.812v-6.188c3.48-0.839 6.72-2.184 9.69-4l4.38 4.375c2.24-1.486 4.29-3.23 6.18-5.125 1.9-1.895 3.64-3.945 5.13-6.187l-4.38-4.375c1.82-2.968 3.16-6.213 4-9.688h6.19c0.53-2.585 0.81-5.261 0.81-8s-0.28-5.415-0.81-8h-6.19c-0.84-3.475-2.18-6.72-4-9.687l4.38-4.375c-1.49-2.243-3.23-4.293-5.13-6.18-1.89-1.89-3.94-3.63-6.18-5.125l-4.38 4.375c-2.97-1.81-6.21-3.16-9.69-4v-6.18c-2.58-0.52-5.26-0.81-8-0.81zm0 20c11.04 0 20 8.96 20 20s-8.96 20-20 20-20-8.96-20-20 8.96-20 20-20z" fill="#949096" stroke="#2c2c2c" stroke-dashoffset="162" stroke-linecap="round" stroke-linejoin="round" stroke-width="4"/><\/g>';

var del_svg = '<g transform="matrix(.94828 0 0 .96584 .62058 .40212)"><circle cx="12" cy="12" r="11.5" fill="#f44336" stroke="#2c2c2c"/><g transform="matrix(.052628 0 0 .052628 -3.7757 -11.107)" fill="#fff"><rect transform="matrix(.7071 -.7071 .7071 .7071 -222.62 340.69)" x="267.16" y="307.98" width="65.545" height="262.18"/><rect transform="matrix(.7071 .7071 -.7071 .7071 398.39 -83.312)" x="266.99" y="308.15" width="65.544" height="262.18"/><\/g><\/g>';

var next_svg = '<g transform="matrix(.046591 0 0 .046558 .96591 .96557)"><circle cx="236.83" cy="236.83" r="236.83" fill="#067852" stroke="#000" stroke-width="20"/><path d="m358.08 216.09c-27.011-27.011-54.018-54.022-81.029-81.033-25.56-25.564-65.08 14.259-39.456 39.883 11.009 11.009 22.019 22.019 33.028 33.032h-130.06c-36.225 0-36.543 56.109-0.404 56.109h130.47l-33.252 33.252c-25.564 25.56 14.259 65.08 39.883 39.456 27.011-27.007 54.018-54.014 81.029-81.025 10.844-10.849 10.549-28.915-0.214-39.674z" fill="#fff"/><\/g>';


function compute_gcd(a, b) {
  return b ? compute_gcd(b, a % b) : a;
}
// Reduce a fraction by finding the Greatest Common Divisor and dividing by it.
function reduce(numerator, denominator) {
  var gcd = compute_gcd(numerator, denominator);
  return [numerator / gcd, denominator / gcd];
}

var prev_page_w = -1,
  prev_page_h = -1;
var page_w = -1,
  page_h = -1;

function clientPageDimension() {
  var html = document.documentElement;
  prev_page_w = page_w;
  prev_page_h = page_h;

  page_w = Math.max(html.clientWidth || 0, window.innerWidth || 0);
  page_h = Math.max(html.clientHeight || 0, window.innerHeight || 0);
  return [page_w, page_h];
}

function clientPageDimensionChanges() {
  if ((prev_page_w < 0 && prev_page_h < 0) || (prev_page_w === page_w && prev_page_h === page_h)) return false;
  return true;
}

var wpi = null;
var vwidth, vheight;

// Get biggest of the width or height of the body and return the corresponding scaled coordinates
// rc[] is always left, top, right, bottom
function biggest_dim(rc) {
  var bodyWidth, bodyHeight;
  [bodyWidth, bodyHeight] = clientPageDimension();
  //var k, kw = bodyWidth / vwidth, kh = 95 * (bodyHeight / vheight) / 100;
  var k, kw = bodyWidth / vwidth,
    kh = bodyHeight / vheight;
  if (kw > kh) k = kw;
  else k = kh;
  return [rc[0] * k, rc[1] * k, rc[2] * k, rc[3] * k];
}

function coord(div) {
  var rc = biggest_dim(div.wallpaper_rect);
  var imarg = parseInt(img_margin.value);
  div.style.left = parseInt(rc[0]) + imarg + "px";
  div.style.top = parseInt(rc[1]) + imarg + "px";
  div.style.width = parseInt(rc[2]) - parseInt(rc[0]) - 2 * imarg + "px";
  div.style.height = parseInt(rc[3]) - parseInt(rc[1]) - 2 * imarg + "px";
  //div.wallpaper_rect=rc;
}

const array_equal = (a, b) => a.length === b.length && a.every((v, i) => v === b[i]);

function trace_image(div) {
  console.log(`Thumbnail image parameters - name:${div.name}, filename:${div.filename}, wallpaper_rect:${div.wallpaper_rect}`);

  var ri = parseInt(div.style.left) + parseInt(div.style.width) + app.left_border + app.right_border;
  var bo = parseInt(div.style.top) + parseInt(div.style.height) + app.top_border + app.bottom_border;
  console.log(`div x, y, w, h, (right,bottom):${div.style.left}, ${div.style.top}, ${div.style.width}, ${div.style.height}, (${ri},${bo})`);
}

function move_image(div, wallpaper_rect) {
  var tb_rc = biggest_dim(wallpaper_rect);
  //console.log(`tb_rc:${tb_rc} - wallpaper_rect:${wallpaper_rect}`);
  var [bodyWidth, bodyHeight] = clientPageDimension();
  if (array_equal(div.wallpaper_rect, wallpaper_rect)) return;
  var tb_w = tb_rc[2] - tb_rc[0],
    tb_h = tb_rc[3] - tb_rc[1];

  if (tb_rc[0] === parseInt(div.style.left) && tb_rc[1] === parseInt(div.style.top) && tb_rc[2] < bodyWidth && tb_rc[3] < bodyHeight) return;
  //console.log("tb_rc[0]:", tb_rc[0], " != div.style.left:", parseInt(div.style.left), "|| tb_rc[1]:", tb_rc[1], " !== div.style.top:", parseInt(div.style.top), " || tb_rc[2]:", tb_rc[2], " >= bodyWidth:", bodyWidth, " || tb_rc[3]:", tb_rc[3], " >= bodyHeight:", bodyHeight);
  //trace_image(div);
  var ol = div.style.left,
    ot = div.style.top,
    or = div.style.right,
    ob = div.style.bottom;
  var imarg = parseInt(img_margin.value);
  div.style.left = parseInt(tb_rc[0] + imarg) + "px";
  div.style.top = parseInt(tb_rc[1] + imarg) + "px";
  div.style.width = parseInt(tb_w - 2 * imarg) + "px";
  div.style.height = parseInt(tb_h - 2 * imarg) + "px";
  div.wallpaper_rect = wallpaper_rect;
  //trace_image(div);
  //console.log(" ");
  if (ol == div.style.left && ot == div.style.top && or == div.style.right && ob == div.style.bottom) return false;
  return true;
}

// Recenter images if necessary to avoid negatives coordinates in the body
function recenter() {
  //if (!clientPageDimensionChanges()) return;
  var max_right = 0,
    max_bottom = 0;
  var hor_push = 0,
    vert_push = 0;

  // Look for negative offset
  for (var i = 0; i < img_divs.length; i++) {
    var ll = parseInt(img_divs[i].style.left);
    var lt = parseInt(img_divs[i].style.top);
    if (ll < 0) hor_push += -ll;
    if (lt < 0) vert_push += -lt;
  }

  // Force negative offsets to positive
  for (var i = 0; i < img_divs.length; i++) {
    var div = img_divs[i];
    var ll = parseInt(div.style.left) + hor_push,
      lt = parseInt(div.style.top) + vert_push;
    div.style.left = parseInt(ll) + "px";
    div.style.top = parseInt(lt) + "px";

    lr = ll + parseInt(div.style.width);
    if (max_right < lr) max_right = lr;
    lb = lt + parseInt(div.style.height);
    if (max_bottom < lb) max_bottom = lb;
  }

  return [max_right, max_bottom];
}

var resizing = false;

function resize_images(p_moved_image = false) {

  if (wpi !== null || !resizing) {
    resizing = true;
    //console.log("Resizing");

    for (var i = 0; i < img_divs.length; i++) {
      var div = img_divs[i];
      if (!p_moved_image)
        coord(div);
    }

    // Recenter images if necessary
    /* var [max_right, max_bottom] = */
    if (!p_moved_image) recenter();
    //console.log("End resizing");
    resizing = false;
  }

  var [bodyWidth, bodyHeight] = clientPageDimension();
}

function filename(path) {
  return path.split('\\').pop().split('/').pop();
}

async function reject_wallpaper(div) {
  if (rempic.checked) {
    console.log(`Not Deleting '${div.filename}'`);
  } else {
    //console.log(`Moving '${div.filename}' to the folder '${rejectfolder.value}'`);

    if (!(await fs.exists(rejectfolder.value))) fs.mkdir(rejectfolder.value);
    if (!(await fs.rename(div.filename, rejectfolder.value + '/' + filename(div.filename)))) {
      console.log(`Something went wrong moving ${div.filename} to ${rejectfolder.value}, error: ${fs.last_error}`);
    }
  }

  // Cycle pour retrouver le papier peint suivant
  for (var i = 0; i < img_divs.length; i++) {
    //console.log(`Cycle ${i} pour retrouver le papier peint suivant`);
    await next_wallpaper_infos();
  }

  await update_images();
}

async function update_image(div, name, file, wallpaper_rect) {
  if (darkmode.checked) {
    div.style.backgroundColor = darken_rgb(bgcolor.value);
  } else {
    div.style.backgroundColor = bgcolor.value;
  }

  div.name = name;
  div.filename = file;
  div.wallpaper_rect = wallpaper_rect;

  coord(div);
  //  get jpeg image file in base64 text
  var data = "data:image/jpeg;base64," + (await fs.read_to_base64(file));
  div.children[0].setAttribute('src', data);
  //console.log(`name: ${name}, file: ${file}`);
  div.children[1].innerHTML = name.substr(7);
  div.children[3].innerHTML = "&nbsp;" + file.replace(/^.*[\\/]/, '') + "&nbsp;";
}

async function create_image(name, file, wallpaper_rect) {
  var div = document.createElement('div');
  //  div.style.border = "1px solid black";
  //div.style.backgroundColor = "#0078d4"; // "#0078d4" : main screen background,  "#dadada" : secondart screen background
  div.style.zIndex = 2;
  div.style.position = "absolute";
  div.style.display = "flex";
  div.style.justifyContent = "center";
  div.style.alignItems = "center";
  div.style.borderRadius = "4px";

  div.classList.add("bg_container");

  var img = document.createElement('img');
  div.appendChild(img);
  img.style.maxWidth = "98%";
  img.style.maxHeight = "98%";

  var m_num = document.createElement('p');
  div.appendChild(m_num);
  m_num.style.backgroundColor = "rgba(0, 0, 0, 0.75)";
  m_num.style.color = "white";
  m_num.style.position = "absolute";
  m_num.style.bottom = "-10px";
  m_num.style.left = "10%";
  m_num.style.width = "22px";
  m_num.style.textAlign = "center";
  m_num.style.border = "1px solid LightGreen";
  m_num.classList.add("switch_vis");
  m_num.style.visibility = "hidden";

  var delb = add_svg_button(div, "", reject_wallpaper, "Reject this background file and cycle until next one for this screen.", del_svg);
  delb.style.position = "absolute";
  delb.style.bottom = "0px";
  delb.style.right = "10%";
  delb.classList.add("switch_vis");
  delb.style.visibility = "hidden";

  var bg_file = document.createElement('p');
  div.appendChild(bg_file);
  bg_file.style.backgroundColor = "rgba(0, 0, 80, 0.9)";
  bg_file.style.color = "white";
  bg_file.style.position = "absolute";
  bg_file.style.bottom = "10%";
  //bg_file.style.width = "90%";
  bg_file.style.textAlign = "center";
  bg_file.style.border = "1px solid yellow";
  bg_file.classList.add("switch_vis");
  bg_file.style.visibility = "hidden";

  await update_image(div, name, file, wallpaper_rect);
  m_num.innerHTML = div.name.substr(7);
  bg_file.innerHTML = "&nbsp;" + div.filename.replace(/^.*[\\/]/, '') + "&nbsp;";
  document.body.appendChild(div);
}

var last_vwidth = 0,
  last_vheight = -1;
var vwidth = 0,
  vheight = 0;
var tbw = app.w,
  tbh = app.h,
  last_tbw, last_tbh;
// Check aspect ratio changes and recompute "virtual screen" dimensions if necessary
// Return true if aspect ratio has not changed else false
async function same_aspect_ratio() {
  var out_x_or_y = 100000;

  // Try to compute virtual screen coordinate if more than 1 monitor with wallpaper
  var vleft = out_x_or_y,
    vtop = out_x_or_y,
    vright = -out_x_or_y,
    vbottom = -out_x_or_y;

  for (var k in wpi) {
    var cwi = wpi[k];
    var rc = cwi["rect"];
    // Does not consider monitor with empty rect
    if (rc.length !== 4) continue;
    if (rc[0] < vleft) vleft = rc[0];
    if (rc[1] < vtop) vtop = rc[1];
    if (rc[2] > vright) vright = rc[2];
    if (rc[3] > vbottom) vbottom = rc[3];
  }

  vwidth = vright - vleft;
  vheight = vbottom - vtop;

  if (last_vwidth !== vwidth || last_vheight !== vheight) {
    var [last_arn, last_ard] = reduce(last_vwidth, last_vheight);
    var [arn, ard] = reduce(vwidth, vheight);

    if (last_arn !== 0 && last_ard != 1) {
      //console.log(`Virtual desktop resolution has changed from (${last_vwidth}, ${last_vheight}) to (${vwidth}, ${vheight})`);
      last_ar = last_vwidth / last_vheight;
      ar = vwidth / vheight;

      var kw = parseFloat(vwidth) / parseFloat(last_vwidth);
      var kh = parseFloat(vheight) / parseFloat(last_vheight);
      var tbw = Math.ceil(kw * parseFloat(app.w)); //+(app.left_border+app.right_border);
      var tbh = Math.ceil(kh * parseFloat(app.h)); //+(app.top_border+app.bottom_border);
      //console.log(`Aspect ratio has changed from ${last_arn}/${last_ard} to ${arn}/${ard} (multiplier coefficients x:${kw} and y:${kh}) and then app size changes from (${app.w},${app.h}) to (${tbw},${tbh}))`);
      await app.set_size(tbw, tbh);
      //console.log(`client size is (${clientPageDimension()})`);
    }

    if (vwidth !== last_vwidth) last_vwidth = vwidth;
    if (vheight !== last_vheight) last_vheight = vheight;

    return false;
  }

  return true;
}

var first_move = true,
  showingConfig = false;
var upto_id;
async function update_images() {
  if (showingConfig) return;

  // Check if new wallpaper images have appeared
  wpi = await win.wallpapers_info();
  var n_monitors = Object.keys(wpi).length;

  var moved_image = false;

  if (n_monitors > 0) {
    // 1st pass to compute new virtual screen dimensions and check aspect ratio
    await same_aspect_ratio();

    // 2nd pass to update and display the images scaled to the body dimensions, if necessary
    for (var k in wpi) {
      var cwi = wpi[k];
      var new_filename = cwi["file"];
      var actual_div = get_image_by_name(k);
      if (actual_div == null) continue; // Should not happen ...
      var wp_rc = cwi["rect"];
      // Remove wallpaper thumbnail if its monitor has empty rect
      if (wp_rc.length !== 4) {
        actual_div.remove();
        continue;
      }

      // If same filename for monitor and image k then only check if we have to move the image
      if (actual_div.filename === new_filename) {
        if (move_image(actual_div, wp_rc)) moved_image = true;
      } else {
        // Finally update the image
        //console.log(`Updating [${k.substr(7)}]`);
        await update_image(actual_div, k, new_filename, wp_rc);
      }
    }
  }

  resize_images(moved_image);

  // bottom, right side adjustment ...
  if (moved_image || first_move) {
    var biggest_ri = 0,
      biggest_bo = 0;
    for (i = 0; i < img_divs.length; i++) {
      var ri = parseInt(img_divs[i].style.left) + parseInt(img_divs[i].style.width) + app.left_border + app.right_border;
      var bo = parseInt(img_divs[i].style.top) + parseInt(img_divs[i].style.height) + app.top_border + app.bottom_border;
      if (ri > biggest_ri) biggest_ri = ri;
      if (bo > biggest_bo) biggest_bo = bo;
    }

    if (biggest_ri > app.w) new_app_w = biggest_ri;
    else new_app_w = app.w;

    if (biggest_bo > app.h) new_app_h = biggest_bo;
    else new_app_h = app.h;

    first_move = false;
    await app.set_size(new_app_w, new_app_h);
  }

  upto_id = setTimeout(update_images, 200);
}

function tools_visibility(hide) {
  document.querySelectorAll('.switch_vis').forEach(function(node) {
    //console.log(`visibility ${node.style.visibility}`);
    if (hide || typeof node.style.visibility == 'undefined' || node.style.visibility.length == 0 || node.style.visibility == 'visible') node.style.visibility = 'hidden';
    else node.style.visibility = 'visible';
  });
}

function on_mouse(event) {
  //  console.log(`event (${event.pageX},${event.pageY}) -- ${event.type}`);
  tools_visibility(event.type == 'mouseleave');
}

function on_click(PointerEvent) {
  //  console.log(`Click (${PointerEvent.pageX},${PointerEvent.pageY})`);
  tools_visibility();
}

function add_svg_button(par, initStyle, click, title, svg) {
  var b = document.createElement('button');
  b.style = initStyle;
  b.addEventListener('mouseover', () => {
    b.style.filter = 'brightness(1.5)';
  });
  b.addEventListener('mouseleave', () => {
    b.style.filter = 'brightness(1.0)';
  });

  b.addEventListener('click', () => {
    click(par);
  }, true);

  b.title = title;
  //b.style.opacity = 0.5;
  b.style.background = "none";
  b.style.border = "none";
  b.innerHTML = '<svg width="24" height="24">' + svg + '<\/svg>';
  par.appendChild(b);
  return b;
}

async function set_img_divs(on = true) {
  tools_visibility(on);

  if (on) {
    await app.set_size(250, 200, 1);
    upto_id = setTimeout(update_images, 200);
    document.body.addEventListener("resize", resize_images);
    document.body.addEventListener("mouseenter", on_mouse);
    document.body.addEventListener("mouseleave", on_mouse);
    //document.body.addEventListener("click", on_click);
  } else {
    clearTimeout(upto_id);
    document.body.removeEventListener("resize", resize_images);
    document.body.removeEventListener("mouseenter", on_mouse);
    document.body.removeEventListener("mouseleave", on_mouse);
    //document.body.removeEventListener("click", on_click);
  }
}

async function runConfig() {
  //  document.removeEventListener("keyup", exit_on_esc);
  await showConfig();
  //  document.addEventListener("keyup", exit_on_esc);
}

async function create_images() {
  wpi = await win.wallpapers_info();
  var n_monitors = Object.keys(wpi).length;

  if (n_monitors > 0) {
    // 1st pass to compute virtual screen dimensions and check aspect ratio
    await same_aspect_ratio();

    // 2nd pass to create and display the images scaled to the body dimensions
    for (var k in wpi) {
      var cwi = wpi[k];
      var wp_rc = cwi["rect"];
      // Does not create wallpaper thumbnail if its monitor has an empty rect
      if (wp_rc.length !== 4) continue;
      var file = cwi["file"];
      await create_image(k, file, wp_rc);
    }
  }

  var tool_div = document.createElement('div');
  tool_div.style.zIndex = 8;
  img_divs = document.getElementsByClassName('bg_container');
  tool_div.style.position = "absolute";
  tool_div.style.left = "40px";
  tool_div.style.top = "10px";
  tool_div.classList.add("switch_vis");
  tool_div.style.visibility = "hidden";

  tool_div.classList.add("switch_vis");
  add_svg_button(tool_div, "20px", runConfig, "Configuration", cnf_svg);
  add_svg_button(tool_div, "140px", next_wallpaper_infos, "Next background", next_svg);
  document.body.appendChild(tool_div);
  set_img_divs();
}

function remove_images() {
  if (wpi !== null) {
    for (var i = 0; i < img_divs.length; i++) {
      var div = img_divs[i];
      div.remove();
    }
  }
}

function get_image_by_name(name) {
  for (var i in img_divs) {
    if (img_divs[i].name === name) {
      return img_divs[i];
    }
  }

  return null;
}

async function loadConfig() {
  var l_wbgco = localStorage.getItem("winbgcolor");
  if (l_wbgco === null) wbgcolor.value = "#ffffff";
  else wbgcolor.value = l_wbgco;

  var l_bgco = localStorage.getItem("bgcolor");
  if (l_bgco === null) bgcolor.value = "#0078d4";
  else bgcolor.value = l_bgco;

  var l_dmod = localStorage.getItem("darkmode");
  if (l_dmod === null || l_dmod === "false") darkmode.checked = false;
  else darkmode.checked = l_dmod;

  var l_imarg = localStorage.getItem("img_margin");
  if (l_imarg === null) img_margin.value = 2;
  else {
    if (l_imarg < 1) img_margin.value = 1;
    else if (l_imarg > 20) img_margin.value = 20;
    else img_margin.value = l_imarg;
  }

  var l_rejmet = localStorage.getItem("rejectmethod");
  if (l_rejmet === null) l_rejmet = "move";
  setRejMet(l_rejmet);


  var l_fld = localStorage.getItem("rejectfolder");
  if (l_fld === null) {
    var l_usrp = await env.get("USERPROFILE");
    if (l_usrp === "No value found for USERPROFILE") l_usrp = "C:\\Users\\Public"; // Should not happen ...
    rejectfolder.value = l_usrp + "\\Pictures\\RejectedWallpapers";
  } else rejectfolder.value = l_fld;

  //console.log(`loadConfig -- wbgcolor: ${wbgcolor.value}, bgcolor: ${bgcolor.value}, darkmode: ${darkmode.checked}, l_rejmet: ${l_rejmet}, rejectfolder: ${rejectfolder.value}`);
}

async function saveConfig() {
  localStorage.setItem("winbgcolor", wbgcolor.value);
  localStorage.setItem("bgcolor", bgcolor.value);
  if (darkmode.checked) localStorage.setItem("darkmode", "true");
  else localStorage.setItem("darkmode", "false");
  localStorage.setItem("rejectfolder", rejectfolder.value);

  if (img_margin.value < 1) localStorage.setItem("img_margin", 1);
  else if (img_margin.value > 20) localStorage.setItem("img_margin", 20);
  else localStorage.setItem("img_margin", img_margin.value);

  var l_rejmet;
  if (rempic.checked) l_rejmet = "delete";
  else l_rejmet = "move";
  localStorage.setItem("rejectmethod", l_rejmet);

  //console.log(`saveConfig -- bgcolor: ${bgcolor.value}, darkmode: ${darkmode.checked}, rejectmethod: ${l_rejmet}, rejectfolder: ${rejectfolder.value}`);
  //localStorage.clear();
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
          await app.bring_to_top(win_id);
          app.exit();
        })();
      }
    }

    function clean_exit() {
      bc.close();
      app.exit();
    }

    function exit_on_esc() {
      if (event.keyCode === 27) clean_exit();
    }
    document.addEventListener("keyup", exit_on_esc);


    // Only run under Windows
    if (app.sysname === "Windows") {
      await app.set_size(250, 200, 1);
      await app.show();
      loadConfig();
      await create_images();
      setDarkMode(darkmode.checked);
    } else(async () => {
      await app.set_size(300, 120);
      await app.set_size(300, 120, 3);
      await app.center();
      document.body.innerHTML = "<center>Sorry, this app only run under Windows<br /><br /><button style='width:6em' onclick='app.exit()'>  OK  </button></center>";
      await app.show();
    })();
  }
}