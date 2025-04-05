// Tries to correct missing keywords in the javascript command line and run it
async function my_eval() {
    cmd_line = document.getElementById("cmd_line");
    var ln = cmd_line.value.split("//")[0].trim();
    if (ln.startsWith("*")) ln = ln.substring(1);
    //if (!ln.endsWith(")")) ln+="()";
    console.log("LINE: [" + ln + "], " + typeof ln + ", " + ln.slice(-1));

    var has_async = ln.includes("async");
    var has_displ = (ln.includes("print(") || ln.includes("println("));
    var has_a_func = false;

    var new_line = "";
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
                aws = s.substring(ii - 6, ii);
                //console.log("AWAIT in "+s+"?: "+aws+", ("+(ii-6)+","+ii+")");
                if (ii < 6 || s.substring(ii - 6, ii) !== "await ") {
                    s = s.slice(0, ii) + "await " + s.slice(ii);
                    //console.log(`Adding 'await ' to ${s}`);
                    ii += 6; // Pointe toujours la position de début de la fonction dans s
                }

                // Eventuellement ici on peut enrober avec print() ou println() plutôt que toute la chaine ...

                has_a_func = true;
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
            s = "println(" + s + ')';
        }

        new_line += s;
        if (nel < vs.length - 1) new_line += ';';
        nel++;
    });


    if (!new_line.includes("print") && !new_line.includes("println")) {
        tmpvname = "vname_" + new Date().getTime();
        console.log(tmpvname);
        new_line = tmpvname + '=' + new_line + '; if (' + tmpvname + ' != "undefined") println(' + tmpvname + ') else println("");'
        //    new_line="println("+new_line+")";
    }

    if (!has_async && has_a_func) {
        new_line = '(async()=>{ ' + new_line + '; })()';
        console.log(new_line);
    } else {
        if (has_a_func && new_line.slice(-1) !== ')') new_line += "()";
        console.log(new_line);
    }

    eval(new_line);
    cmd_line.value = "";
}

document.getElementById("cmd_line").addEventListener("keyup", ({
    key
}) => {
    if (key === "Enter") my_eval();
});

Latin_lowercase = "abcdefghijklmnopqrstuvwxyz";
Latin_uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
Latin_variant = "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";
Arabic_alphabet = "ابتثجحخدذرزسشصضطظعغفقكلمنهوي";
Chineese = "啊爱安暗按八把爸吧白百拜班般板半办帮包保抱报爆杯北被背备本鼻比笔避必边便遍辨变标表别病并补不部布步才材采彩菜参草层曾茶察查差产长常场厂唱车彻称成城承程吃冲虫出初除楚处川穿传船窗床创春词此次从村存错答达打大带待代单但淡蛋当党导到道的得灯等低底地第弟点典电店掉调丁定冬东懂动都读独度短断段对队多朵躲饿儿而耳二发乏法反饭范方房防访放非飞费分坟份风封夫服福府父副复富妇该改概敢感干刚钢高搞告哥歌革隔格个给跟根更工公功共狗够构姑古骨故顾固瓜刮挂怪关观官馆管惯光广规鬼贵国果过还孩海害含汉好号喝河和何合黑很恨红后候呼忽乎湖胡虎户互护花华划画化话怀坏欢环换黄回会婚活火或货获机鸡积基极及集级急几己寄继际记济纪技计季家加假架价间简见建健件江将讲交饺脚角叫教较接街阶结节解姐介界今金斤仅紧近进尽京经精睛景静境究九酒久就旧救居局举句具据剧拒觉绝决军开看康考靠科可课刻客肯空孔口苦哭快筷块况困拉来浪劳老乐了累类冷离李里理礼立丽利历力例连联脸练凉两辆亮量谅疗料烈林零〇领另留流六龙楼路旅绿虑论落妈马吗买卖满慢忙毛么没美每门们";

var func_examples = [
    'help() // Some help and the list of available functions for this app.',
    'io_write("Input a string: ");; println(io_readln())',
    'println([...Chineese].length)',
    'for(i=0;i<Chineese.length;i+=25) println((i.toString()+"-"+(i+25).toString()).padStart(7)+": "+Chineese.slice(i, i+25)) // display 900 chineese ideograms by lines of 25',
    'println("<div style=\\"text-wrap:wrap;width:50ch;\\">"+Chineese+"</div>") // display 900 chineese ideograms by lines of 25',
    'win_regget("Software\\\\webview-app", "test_var", "def_val")',
    'pos=await app_pos(); println(`${pos.x},${pos.y}`) // display window position',
    'app_icon("../wiki_contrib/app.ico") // Change app icon',
    'geo=await app_geometry(); println(`${geo.x},${geo.y},${geo.w},${geo.h}`) // display window geomtry',
    'fs_read("name💥🧸🤖🔥=安暗按八=ثج.txt") // Read a text file',
    'fs_stat("name💥🧸🤖🔥=安暗按八=ثج.txt") // Display file status',
    'fs_mkdir("dir基极及") // Create a subdirectory to the current one',
    'fs_current_path("dir基极及") // Go to the named subdirectory',
    'io_write(Chineese) // Display chineese ideograms to stdout',
    'JSON.stringify(fs_ls("/Videos/4-Films-VO", true, false, true)).replace(/{"path/g, "\\n  {\\"path").replace(/\\]/, "\\n]").replace(/","/g, "\\"\\n   \\"")',
    '`For Windows: ${env_get("COMPUTERNAME")}@${env_get("USERNAME")}:${fs_current_path()}`',
    '`For Posix: ${env_get("HOSTNAME")}@${env_get("USER")}:${fs_current_path()}`',
    '${(app_info()).replace(/,/g, "\\n")}',
    'gui.msgbox("Message", "Titre", 1)',
    'println("! #$%&*+,-./0124365789:;<=>?@' + Latin_uppercase + '[\]^_`' + Latin_lowercase + '{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿' + Latin_variant + '")',
    'println("Latin ' + Latin_lowercase + '\\nArabe ' + Arabic_alphabet + '\\nChinois ' + Chineese + '")',
];

function fill_func_list() {
    func_examples.forEach(function(item, index) { // Add examples for completion
        var option = document.createElement('option');
        var cmd = item.split("//")[0].trim();
        if (cmd.startsWith("*")) cmd = cmd.substring(1);
        if (!cmd.endsWith(")")) cmd += "()";
        option.value = cmd;
        cmd_list.appendChild(option);
        if (index == 0) cmd_line.value = item;
    });

    func_list.forEach(function(item, index) { // Add all the functions for completion
        var option = document.createElement('option');
        var cmd = item.split("//")[0].trim();
        if (cmd.startsWith("*")) cmd = cmd.substring(1);
        if (!cmd.endsWith(")")) cmd += "()";
        option.value = cmd;
        cmd_list.appendChild(option);
    });
}

async function do_load() {
    //if (typeof navigator.appVersion === 'string') console.log(navigator.appVersion);
    //if (typeof navigator.userAgentData !== 'undefined' && typeof navigator.userAgentData.platform === 'string') console.log(navigator.userAgentData.platform);
    hname = await env_get("COMPUTERNAME");
    if (hname === "") hname = await env_get("HOSTNAME");
    if (is_windows) {
        var msys_ubin = "",
            msys_dir = "";
        if (hname === "PC-DENIS") {
            msys_dir = 'D:\\UnixTools\\msys64\\';
        } else {
            msys_dir = 'C:\\Users\\S0125790\\MyApp\\msys64\\';
        }

        pth = await env_get("PATH");
        if (!pth.includes("usr\\bin")) {
            console.log("Including usr\\bin");
            env_ins("PATH", msys_dir + "usr\\bin;");
        }
        if (!pth.includes("mingw64\\bin")) {
            console.log("Including mingw64\\bin");
            env_ins("PATH", msys_dir + "mingw64\\bin;");
        }

        // console.log(hname+"==>"+msys_ubin);
    }

    cmd_line.focus();
    // Auto select
    //cmd_line.select();
    // Set the cursor to the start
    cmd_line.setSelectionRange(0, 0);

    document.addEventListener('keydown', logAnyKey);
    console.log("Event key pressed F2");
}
/*
var ctrl_down=false;
function logAnyKey() { 
  if (event.ctrlKey) ctrl_down=true;
  else if (event.keyCode === " ") { // F2 key
    fill_func_list();
    cmd_line.value="";
    console.log("F2 key pressed");
    document.getElementById("cmd_line").list="cmd_list";
  }  else ctrl_down=false;
}


*/
function logAnyKey() {
    console.log("Key pressed " + event.keyCode);
    if (event.ctrlKey && event.keyCode === " ") { // F2 key
        fill_func_list();
        cmd_line.value = "";
        console.log("F2 key pressed");
        document.getElementById("cmd_line").list = "cmd_list";
    }

}

let but_style = 'style="height:16px; padding:0; background:none; border:none; cursor:pointer; color:blue; text-decoration:underline;"';

function mk_url(label, url) {
    ret = `<input type="button" ${but_style} onclick="location.href='${url}'" value="${label}"/>`;
    //console.log(ret);
    return ret;
}

//let example_string='cls();\nprintln(`Reworked content of the "PATH" environment variable (path separators replaced by newlines):\\n${(env_get("PATH")).replace(/'+(is_windows?';':':')+'/g, "\\n")}`);';

let example_string = 'cls();;\nprintln("<u>Reworked content of the \'PATH\' environment variable (path separators replaced by newlines):</u>");;\nprintln(`${(env_get("PATH")).replace(/' + (is_windows ? ';' : ':') + '/g, "\\n")}`)';
let example_url = `<button ${but_style} onclick="cmd_line.value=example_string; my_eval(example_string)">directly here !<\/button>`;
let tlit_url = "https://developer.mozilla.org/docs/Web/JavaScript/Reference/Template_literals";
let prom_url = "https://developer.mozilla.org/docs/Web/JavaScript/Reference/Global_Objects/Promise";
let async_url = "https://developer.mozilla.org/docs/Web/JavaScript/Reference/Statements/async_function";
let await_url = "https://developer.mozilla.org/docs/Web/JavaScript/Reference/Operators/await";
async function help(obj = output) {
    print(`Below is a list of all the (mostly) asynchronous added functions of this webview app.
You can test them through the fields 'Javascript line' or 'Javascript file'.
There eventual output may be encapsulated in a ${mk_url('template literal', tlit_url)} within the print() or println() functions.
Something like that:
<u><span style=\"font-family:monospace\">${example_string}<\/span></u>
Try to copy/paste the 3 previous line in the 'Javascript line' field and click on the 'Run the line !' button, or ${example_url}
Notice that you can also use the devtools console for all that (F12), it may be more handy or accurate, sometimes.
But you'll have to deal with the asynchronous context by using ${mk_url('promises', prom_url)} or the ${mk_url('await', await_url)} keyword in an ${mk_url('asynchronous function', async_url)}.
<u>Now the list of functions:<\/u>
<i>Async ones have their name prefixed with a '*'<\/i>\n`);
    s = await window.app_help();
    println(func_help.join('\r\n'), false);
}

window.addEventListener("load", do_load);
document.addEventListener("keyup", exit_on_esc);