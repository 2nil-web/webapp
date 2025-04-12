async function sleep(nsec) {
  await new Promise(r => setTimeout(r, nsec * 1000));
}

function exec_cmd_no_return(target, cmd_val, duration) {
  if (target.disabled === true) {
    target.disabled = false;
  }

  cmd_line.focus();
}

// Ajoute une parenthése ouvrante et un fermante s'il n'en trouve pas dans la chaine
function add_parenth(s) {
  s = s.trim(); // ça à certainement déjà était fait mais bon ...

  var add_open = true,
    add_close = true;

  for (let i = 0; i < s.length; i++) {
    if (s.charAt(i) === '(') add_open = false;
    if (!add_open && s.charAt(i) === ')') add_close = false;
  }

  if (add_open) s += '(';
  if (add_close) s += ')';

  //console.log(s);
  return s;
}

function looseJsonParse(obj) {
  return Function('"use strict";return (' + obj + ")")();
}

// Tries to correct missing keywords in the javascript command line and run it
async function my_eval_old() {
  var ln = cmd_line.value.split("//")[0].trim();
  if (ln.startsWith("*")) ln = ln.substring(1);
  //console.log("LINE: ["+ln+"]");

  var new_line;

  if (false) {
    var has_async = ln.includes("async");
    var has_displ = (ln.includes("print(") || ln.includes("println("));
    var has_a_func = false;

    new_line = "";
    vs = ln.split(/[;;\n]+/);
    nel = 0;

    vs.forEach(function(s) {
      var this_one_has_func = false;
      s = s.trim();
      //console.log(`Searching in ${s}`);
      for (func of func_list) {
        func = func.split("()")[0].trim();
        // Eventually remove the star indicating async func
        if (func.startsWith('*')) func = func.substring(1);
        func = func.trim();
        const func_re = new RegExp("\\b" + func + "\\b");
        //console.log("FUNC: "+func, " FUNC_RE: "+func_re);

        s1 = s;

        var ii = 0;
        for (;;) {
          s1 = s.slice(ii);
          i = s1.search(func_re);
          if (i === -1) break;
          ii += i; // Pointe la position de début de la fonction dans s
          //console.log(`Found match of function '${func}' in string '${s}' at position:${ii}`);

          // S'il n'y a pas déjà 'await ' avant le début de la fonction, alors on l'insert
          if (ii < 6 || s.substring(ii - 6, ii) !== "await ") {
            aws = s.substring(ii - 6, ii);
            console.log("AWAIT in " + s + "?: " + aws + ", (" + (ii - 6) + "," + ii + ")");
            s = s.slice(0, ii) + "await " + s.slice(ii);
            console.log(`Adding 'await ' to ${s}`);
            ii += 6; // Pointe toujours la position de début de la fonction dans s
          }

          if (has_a_func) has_a_func = false;
          else has_a_func = true;

          this_one_has_func = true;
          //console.log(`S: [[${s}]], opening parenthese at ${ii+func.length} ? ==> ${s.charAt(ii+func.length)}`);
          inc = +func.length;
          // Insert des parenthéses ouvrante et fermante si on n'en trouve pas une ouvrante à la fin de la fonction ...
          if (s.charAt(ii + func.length) != '(') {
            s = s.substr(0, ii + func.length) + "()" + s.substr(ii + inc);
            console.log(`Adding opening parenthese to S: [[${s}]]`);
            inc += 2;
          }

          ii += inc; // Pointe à la fin de la fonction après les parenthéses ouvrante et fermante, s'il y a lieu ...
        }
      }

      if (!has_displ && this_one_has_func) {
        if (s.includes("\${") && !s.includes('`')) s = '`' + s + '`';
        s1 = s;
        tmpvname = "vname_" + new Date().getTime();
        s = tmpvname + '=' + s1 + '; if (typeof ' + tmpvname + ' !== "undefined") println(' + tmpvname + ')';
        //console.log(tmpvname);
        //s="println("+s+')';
      }

      new_line += s;
      if (nel < vs.length - 1) new_line += ';';
      nel++;
    });

    /*
      if (!new_line.includes("print") && !new_line.includes("println")) {
        tmpvname="vname_"+new Date().getTime();
        new_line=tmpvname+'='+new_line+'; if ('+tmpvname+' != "undefined") println('+tmpvname+')';
      }
    */
    if (!has_async && has_a_func) {
      new_line = '(async()=>{ ' + new_line + '; })()';
      //console.log(new_line);
    } else {
      if (has_a_func && new_line.slice(-1) !== ')') new_line += "()";
      //jscript_line.disabled = false;
    }
  } else {
    new_line = ln;
  }

  //cmd_line.value=new_line;
  console.log("Before " + new_line);
  document.removeEventListener("keyup", exit_on_esc);
  jscript_line.disabled = true;
  await eval(new_line);
  console.log("After " + new_line);
  jscript_line.disabled = false;
  cmd_line.focus();
  await sleep(0.8);
  document.addEventListener("keyup", exit_on_esc);
  //looseJsonParse("{"+new_line+"}");

  cmd_line.focus();
  return
}

const wordInString = (s, word) => new RegExp('\\b' + word + '\\b', 'i').test(s);

async function my_eval() {
  var line = cmd_line.value.split("//")[0].trim();
  if (line.startsWith("*")) line = line.substring(1);

  //if (line.includes("await") {
  if (line.match(/(^|\W)await($|\W)/)) {
    line = '(async()=>{ ' + line + '; })()';
  }

  //console.log("Running: "+line);
  jscript_line.disabled = true;
  document.removeEventListener("keyup", exit_on_esc);

  console.log(line);
  await eval(line);
  await sleep(0.8);

  document.addEventListener("keyup", exit_on_esc);
  jscript_line.disabled = false;
  cmd_line.focus();
}

document.getElementById("cmd_line").addEventListener("keyup", ({
  key
}) => {
  if (key === "Enter") my_eval();
});

document.getElementById("jscript_line").addEventListener("click", async () => {
  my_eval();
});

var cmd_file_obj;
document.getElementById("jscript_file").addEventListener("click", async () => {
  //console.log(OSName);
  if (OSName === "Windows" || OSName === "Linux") {
    if (cmd_file.value !== '') {
      (async () => {
        //console.log("READ:");
        //console.log(cmd_file.value);
        content = await fs.read(cmd_file.value);
        //console.log("CONTENT: "+content);
        if (!content.startsWith("Unable to open the file")) {
          //console.log("RUN:");
          //console.log(content);
          document.removeEventListener("keyup", exit_on_esc);
          await eval(content);
          sleep(1);
          document.addEventListener("keyup", exit_on_esc);
        }
      })();
    } else {
      console.log("No file selected");
    }
  }
});

document.getElementById("select_file").addEventListener("click", async () => {
  // Disable esc_on_exit which may interfer with esc on msg_box
  document.removeEventListener("keyup", exit_on_esc);
  cmd_file_s = await gui.opendlg("*.js;Fichier javascript", ".", "Select a javascript file to run");
  if (cmd_file_s != "")
    cmd_file.value = cmd_file_s;
  // Re-enable esc_on_exit
  await new Promise(r => setTimeout(r, 400));
  document.addEventListener("keyup", exit_on_esc);
});

Latin_lowercase = "abcdefghijklmnopqrstuvwxyz";
Latin_uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
Latin_variant = "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";
Arabic_alphabet = "ابتثجحخدذرزسشصضطظعغفقكلمنهوي";
Chineese = "啊爱安暗按八把爸吧白百拜班般板半办帮包保抱报爆杯北被背备本鼻比笔避必边便遍辨变标表别病并补不部布步才材采彩菜参草层曾茶察查差产长常场厂唱车彻称成城承程吃冲虫出初除楚处川穿传船窗床创春词此次从村存错答达打大带待代单但淡蛋当党导到道的得灯等低底地第弟点典电店掉调丁定冬东懂动都读独度短断段对队多朵躲饿儿而耳二发乏法反饭范方房防访放非飞费分坟份风封夫服福府父副复富妇该改概敢感干刚钢高搞告哥歌革隔格个给跟根更工公功共狗够构姑古骨故顾固瓜刮挂怪关观官馆管惯光广规鬼贵国果过还孩海害含汉好号喝河和何合黑很恨红后候呼忽乎湖胡虎户互护花华划画化话怀坏欢环换黄回会婚活火或货获机鸡积基极及集级急几己寄继际记济纪技计季家加假架价间简见建健件江将讲交饺脚角叫教较接街阶结节解姐介界今金斤仅紧近进尽京经精睛景静境究九酒久就旧救居局举句具据剧拒觉绝决军开看康考靠科可课刻客肯空孔口苦哭快筷块况困拉来浪劳老乐了累类冷离李里理礼立丽利历力例连联脸练凉两辆亮量谅疗料烈林零〇领另留流六龙楼路旅绿虑论落妈马吗买卖满慢忙毛么没美每门们";

var func_examples = [
  'mi=await win.monitors_info(); console.log(mi); for (k in mi) { if (mi[k]["primary monitor"]) mn=k+ " (primary)"; else mn=k; println(`${mn}<br/>&nbsp;Full area: ${mi[k]["area"]},<br/>&nbsp;Working area: ${mi[k]["working area"]}`); }',
  'help() // Some help and the list of available functions for this app.',
  'di=await win.devices_info(); console.log(di); for (k in di) { println(k); }',
  'res=(await fs.stat("not a file 💥🧸🤖🔥=安暗按八=ثج")); if (res === false) println(fs.last_error); else println(res)',
  'res=(await fs.current_path("not a file 💥🧸🤖🔥=安暗按八=ثج")); if (res === false) println(fs.last_error); println(fs.cwd)',
  'println(await fs.ls(".", true))',
  'println(await app.echo("Parm0", "Parm1", "Parm2", "Parm3", "Parm4", "Parm5", "Parm6", "Parm7", "Parm8", "Parm9"))',
  'app.set_title("Title💥🧸🤖🔥=安暗按八=ثج")',
  'println(app.title)',
  'println("CWD : "+fs.cwd); await fs.current_path("name💥🧸🤖🔥=安暗按八=ثج"); println("CWD : "+fs.cwd); console.log("CWD : "+fs.cwd)',
  'println(await fs.ls("dir基极及", true))',
  'println(await fs.stat("dir基极及"))',
  'if (await fs.current_path("dir基极及") == false) println(fs.last_error); else println(fs.cwd)',
  'println(await fs.echo("Chineese_filename_处川穿传船窗床创春词此.txt"))',
  'println(await fs.exists("Chineese_filename_处川穿传船窗床创春词此.txt"))',
  'var h="name💥🧸🤖🔥=安暗按八=ثج.txt"; console.log(h); var s=h.fromHtmlEntities(); console.log(s); println(await fs.stat(h))',
  'var fn="name💥🧸🤖🔥=安暗按八=ثج.txt"; console.log(fn); println(await fs.stat(fn))',
  'println(await fs.stat("name💥🧸🤖🔥=安暗按八=ثج.txt"))',
  'println(await fs.stat(".")) // Display file status',
  'println(await fs.ls("dir基极及", true, false, true))',
  'println(await fs.current_path("dir基极及"))',
  'println(await fs.absolute("name💥🧸🤖🔥=安暗按八=ثج.txt")) // Display file status',
  'println(`${env.path.replace(/' + (app.sys_info == "Windows" ? ';' : ':') + '/g, "\\n")}`)',
  'await app.set_title("##TUTU##"); println(app.title)',
  'await app.set_icon("../backup/app.ico"); println(app.icon)',
  'println("CWD : "+fs.cwd)',
  'await fs.current_path(".."); println("CWD : "+fs.cwd)',
  'await env.set_path("C:\\\\Windows"); println("env.path: "+env.path)',
  'await env.ins_path("TOTO"); println(env.path)',
  'println(await gui.opendlg("*.c,*.cpp,*.c++,*.cc;C/C++ files|*.html,*.js,*.css;Web files (HTML, Javascript and CSS)", ".."))',
  'println(await fs.read("name💥🧸🤖🔥=安暗按八=ثج.txt")) // Read a text file',
  'println(await fs.stat("name💥🧸🤖🔥=安暗按八=ثج.txt")) // Display file status',
  'fs.mkdir("dir基极及") // Create a subdirectory to the current one',
  'println(`(${app.x}, ${app.y}, ${app.w}, ${app.h})`)',
  'println(JSON.stringify(app_help("json"), null, 2))',
  'println(app_help())',
  'println(app)',
  'println(await gui.opendlg("*.c,*.cpp,*.c++,*.cc;C/C++ files|*.html,*.js,*.css;Web files (HTML, Javascript and CSS)", ".."))',
  'println(await gui.msgbox("Hello"))',
  'println(await gui.opendlg("*.c,*.cpp,*.c++,*.cc;C/C++ files|*.html,*.js,*.css;Web files (HTML, Javascript and CSS)", ".."))',
  'println(await gui.opendlg("*\\\\,z,*.z"))',
  'println(await gui.folderdlg("", "/"))',
  'println(await gui.savedlg("*.c,*.cpp,*.c++,*.cc;C/C++ files|*.html,*.js,*.css;Web files (HTML, Javascript and CSS)"))',
  'println("<div style=\\"text-wrap:wrap;width:50ch;\\">"+Chineese+"</div>") // display 900 chineese ideograms by lines of 50',
  'io.write("Input a string: ");; println(io.readln())',
  'println([...Chineese].length)',
  'for(i=0;i<Chineese.length;i+=25) println((i.toString()+"-"+(i+25).toString()).padStart(7)+": "+Chineese.slice(i, i+25)) // display 900 chineese ideograms by lines of 50',
  'win.regget("Software\\\\webview-app", "test_var", "def_val")',
  'await app.set_pos(10, 20); println(`${app.x},${app.y}`) // display window position',
  'app.set_icon("../wiki_contrib/app.ico") // Change app icon',
  'fs.current_path("dir基极及") // Go to the named subdirectory',
  'io.write(Chineese) // Display chineese ideograms to stdout',
  'JSON.stringify(await fs.ls("/Videos/4-Films-VO", true, false, true)).replace(/{"path/g, "\\n  {\\"path").replace(/\\]/, "\\n]").replace(/","/g, "\\"\\n   \\"")',
  'println(`For Windows: ${await env.get("COMPUTERNAME")}@${await env.get("USERNAME")}:${await fs.current_path()}`)',
  'println(`For Posix: ${await env.get("HOSTNAME")}@${await env.get("USER")}:${await fs.current_path()}`)',
  'errmsg="No value found for var";hn=await env.get("HOSTNAME");u=await env.get("USER");println(`${hn.startsWith(errmsg)?await env.get("COMPUTERNAME"):hn}@${u.startsWith(errmsg)?await env.get("USERNAME"):u}:${await fs.current_path()}`)',
  //(async()=>{ '+new_line+'; jscript_line.disabled = false; cmd_line.focus(); })()
  '${(app.info).replace(/,/g, "\\n")}',
  'gui.msgbox("Message")',
  'gui.msgbox("Question to be answered by OK or Cancel ?", 1)',
  'gui.msgbox("Question to be answered by Yes or No ?", 2)',
  'println("! #$%&*+,-./0124365789:;<=>?@' + Latin_uppercase + '[\]^_`' + Latin_lowercase + '{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿' + Latin_variant + '")',
  'println("Latin ' + Latin_lowercase + '\\nArabe ' + Arabic_alphabet + '\\nChinois ' + Chineese + '")',
];

function fill_func_list() {
  var cmd_list = document.getElementById("cmd_list");
  func_examples.forEach(function(item, index) { // Add examples for completion
    var option = document.createElement('option');
    var cmd = item.split("//")[0].trim();
    //console.log(`fe[${index}]=${cmd}`);
    //if (cmd.startsWith("*")) cmd=cmd.substring(1);
    if (!cmd.endsWith(")")) cmd += "()";
    option.value = cmd;
    cmd_list.appendChild(option);
    if (index == 0) cmd_line.value = item;
  });

  func_list.forEach(function(item, index) { // Add all the functions for completion
    //console.log(`fl[${index}]=${item}`);
    var option = document.createElement('option');
    var cmd = item.split("//")[0].trim();
    if (cmd.startsWith("*")) cmd = cmd.substring(1);
    if (!cmd.endsWith(")")) cmd += "()";
    option.value = cmd;
    cmd_list.appendChild(option);
  });

  // Add the saved javascript file if there is
  jfile = localStorage.getItem("jfile");
  if (jfile != "") {
    cmd_file.value = jfile;
  }

  set_output_flex(localStorage.getItem("output_wrap") === "true");

  cmd_line.focus();
  // Auto select
  //cmd_line.select();
  // Set the cursor to the start
  cmd_line.setSelectionRange(0, 0);
}

// To remove the eventual * at start of datalist helper
cmd_line.addEventListener('input', function() {
  val = this.value;
  if (val.startsWith("*")) val = val.substring(1);
  this.value = val;
});

let but_style = 'style="height:16px; padding:0; background:none; border:none; cursor:pointer; color:blue; text-decoration:underline;"';

function mk_url(label, url) {
  ret = `<input type="button" ${but_style} onclick="location.href='${url}'" value="${label}"/>`;
  //console.log(ret);
  return ret;
}

//let example_string='cls();\nprintln(`Reworked content of the "PATH" environment variable (path separators replaced by newlines):\\n${(env.get("PATH")).replace(/'+(is_windows?';':':')+'/g, "\\n")}`);';

//let example_string='cls();\nprintln("<u>Reworked content of the \'PATH\' environment variable (path separators replaced by newlines):</u>");\nprintln(`${(await env.get("PATH")).replace(/'+(is_windows?';':':')+'/g, "\\n")}`)';
let example_string = 'cls();println("<u>Reworked content of the \'PATH\' environment variable (path separators replaced by newlines):</u>");println(`${env.path.replace(/' + (is_windows ? ';' : ':') + '/g, "\\n")}`)';
let example_url = `<button ${but_style} onclick="cmd_line.value=example_string; my_eval(example_string)">click directly here !<\/button>`;
let tlit_url = "https://developer.mozilla.org/docs/Web/JavaScript/Reference/Template_literals";
let prom_url = "https://developer.mozilla.org/docs/Web/JavaScript/Reference/Global_Objects/Promise";
let async_url = "https://developer.mozilla.org/docs/Web/JavaScript/Reference/Statements/async_function";
let await_url = "https://developer.mozilla.org/docs/Web/JavaScript/Reference/Operators/await";
async function help(obj = output) {
  print(`Below is a list of all the added variables and functions to this webview app.<br/>
You can test them through the fields 'Javascript line' or 'Javascript file'.<br/>
There eventual output may be encapsulated in a ${mk_url('template literal', tlit_url)} within the print() or println() functions.<br/>
Something like that:<br/>
<u><span style=\"font-family:monospace\">${example_string}<\/span></u><br/>
Try to copy/paste the previous line in the 'Javascript line' field and click on the 'Run the line !' button, or ${example_url}<br/>
Notice that when calling one of these functions, you have to deal with the asynchronous context by using ${mk_url('promises', prom_url)} or the ${mk_url('await', await_url)} keyword in an ${mk_url('asynchronous function', async_url)}.<br/>
Basically if you expect a return from any of these functions you must call it in an asynchronous way.
The only context that could allows you to avoid using asynchronous mode would be by calling a function that you do not expect its return, for instance 'app.set_title("New title"), if you don't need to know the old title'.<br/>
`);
  //s=await window.app.help();
  //console.log(var_help);
  //console.log(func_help);
  s = var_help.join('\r\n');
  if (s.length > 0) s = "<u>The variables:</u><br/><i>Readonly ones have their name prepended with a '-'</i><br/>" + s;
  s += "<br/><u>The functions:</u><br/><i>Threaded ones have their name prepended with a '*'</i><br/>";
  s += func_help.join('\r\n');
  s = s.replace(/:0, /g, "").replace(/:1,/g, "accepts 1 parameter,").replace(/:(\d)+,/g, "accepts $1 parameters,");
  s = s.replace(/:0, /g, "").replace(/:1,/g, "accepts 1 parameter,").replace(/:(\d)+,/g, "accepts $1 parameters,");
  println(s.replace(/\n/g, "<br/>").replace(/\r/g, "<br/>").replace(/<br\/><br\/>/g, "<br/>").replace(/ /g, "&nbsp;"), false);
}

function set_output_flex(wrap) {
  if (wrap) {
    output.style.whiteSpace = "normal";
    localStorage.setItem("output_wrap", "true");
    breaks.value = "Lines are wrapped";
  } else {
    output.style.whiteSpace = "nowrap";
    localStorage.setItem("output_wrap", "false");
    breaks.value = "Lines are not wrapped";
  }

  //console.log(breaks.value);
}

function toggle_output_flex() {
  if (output.style.whiteSpace === "normal") {
    set_output_flex(false);
  } else {
    set_output_flex(true);
  }

}

async function do_load() {
  //if (typeof navigator.appVersion === 'string') console.log(navigator.appVersion);
  //if (typeof navigator.userAgentData !== 'undefined' && typeof navigator.userAgentData.platform === 'string') console.log(navigator.userAgentData.platform);
  hname = await env.get("COMPUTERNAME");
  if (hname === "") hname = await env.get("HOSTNAME");
  if (is_windows) {
    var msys_ubin = "",
      msys_dir = "";
    if (hname === "PC-DENIS-2") {
      msys_dir = 'C:\\UnixTools\\msys64\\';
    } else {
      msys_dir = 'D:\\UnixTools\\msys64\\';
      //msys_dir = 'C:\\Users\\S0125790\\MyApp\\msys64\\';
    }

    pth = await env.get("PATH");
    if (!pth.includes("usr\\bin")) {
      //console.log("Including usr\\bin");
      env.ins("PATH", msys_dir + "usr\\bin;");
    }
    if (!pth.includes("mingw64\\bin")) {
      //console.log("Including mingw64\\bin");
      env.ins("PATH", msys_dir + "mingw64\\bin;");
    }

    //console.log(hname+"==>"+msys_ubin);
  }

  fill_func_list();
}


window.addEventListener("load", do_load);
document.addEventListener("keyup", exit_on_esc);

async function do_print() {
  app.maximize();
  win.printdlg();
}
