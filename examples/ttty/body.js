var term;

async function getprompt() {
    var user, hostname, homedir, cwd, PS1;

    if (typeof app_title === "function") {
        let OSName = client_system();
        cwd = await fs_current_path();

        if (OSName === "Windows") {
            user = await env_get("COMPUTERNAME");
            user = user.replace(/\\/g, "/");;
            hostname = await env_get("USERNAME");
            homedir = await env_get("HOMEPATH");
            homedir = homedir.replace(/\\/g, "/");
            cwd = cwd.substring(2);
            cwd = cwd.replace(/\\/g, "/");
        } else {
            user = await env_get("HOSTNAME");
            hostname = await env_get("USER");
            homedir = await env_get("HOME");
        }

        if (cwd.startsWith(homedir)) cwd = "~" + cwd.substring(homedir.length);
        PS1 = user + '@' + hostname + ':' + cwd + "$ ";
    } else {
        PS1 = "user@ttty:~$ ";
    }

    /*
    console.log("user: "+user);
    console.log("hostname: "+hostname);
    console.log("homedir: "+homedir);
    console.log("cwd: "+cwd);
    console.log("PS1: "+PS1);*/
    var termi = document.querySelector("#terminal");
    term.setPrompt(PS1);
}

var term = document.querySelector("#terminal");

function do_pwd() {

}

var lastdir = ".";
async function chdir(dir = "") {
    var actudir = "";

    if (dir === "-") {
        tmpdir = await fs_current_path();
        actudir = await fs_current_path(lastdir);
        lastdir = tmpdir;
    } else {
        lastdir = await fs_current_path();
        actudir = await fs_current_path(dir);
    }

    console.log("chdir lastdir: " + lastdir + ", actualdir: " + actudir);
    getprompt();
    return actudir;
}


const settings = {
    host: term,
    commands: {
        echo: {
            name: "echo",
            description: "a test command with one echo arg",
            argDescriptions: ["a string to be echoed in console"],
            func: ({
                print
            }, argument) => {
                print(argument)
            }
        },
        pwd: {
            name: "pwd",
            description: "show current directory",
            func: async ({
                print
            }) => {
                print(await fs_current_path())
            }
        },
        exit: {
            name: "exit",
            description: "exit from app",
            func: async () => {
                app_exit()
            }
        },
        app_help: {
            name: "app_help",
            description: "app_help",
            func: async ({
                print
            }) => {
                var res = await app_help();
                //console.log(res.replace(/&#xa;/g, '\n'));
                print(res.replace(/&#xa;/g, '<br>'));
            }
        },
        cd: {
            name: "cd",
            description: "change current directory",
            argDescriptions: ["a path string for the new directory"],
            func: ({
                print
            }, arg) => {
                chdir(arg)
            }
        },
        multiply: {
            name: "multiply",
            description: "Multiply two numbers",
            argDescriptions: ["number one", "number two"],
            func: ({
                print
            }, argv0, argv1) => {
                print(Number(argv0) * Number(argv1))
            }
        }
    }
}

function termEvt(txt, ev) {
    for (p in ev) {
        //console.log('['+txt+'], '+p+": "+ev[p]);
    }
    return true;
}

function onInit(ev) {
    termEvt('init', ev);
}

function onCommand(ev) {
    termEvt('onCommand', ev);
}

function onCommand404(ev) {
    termEvt('onCommand404', ev);
}

function onProcessStart(ev) {
    termEvt('onProcessStart', ev);
}

function onProcessStop(ev) {
    termEvt('onProcessStop', ev);
}

function onProcessInterrupt(ev) {
    termEvt('onProcessInterrupt', ev);
}


function do_load() {
    term = ttty.initTerminal(settings);
    const tterm = document.getElementById('terminal');
    tterm.addEventListener('onInit', onInit);
    tterm.addEventListener('onCommand', onCommand);
    tterm.addEventListener('onCommand404', onCommand404);
    tterm.addEventListener('onProcessStart', onProcessStart);
    tterm.addEventListener('onProcessStop', onProcessStop);
    tterm.addEventListener('onProcessInterrupt', onProcessInterrupt);
    getprompt();
    var termInput = document.getElementsByClassName('terminal')[0].getElementsByTagName('input')[0];
    //  termInput.style.background="red";
    //  termInput.style.color="yellow";
    termInput.focus();
    window.addEventListener("keydown", function(evt) {
        if (evt.keyCode === 13) {
            /*
            var ti = document.getElementsByClassName('terminal')[0].getElementsByTagName('input')[0];
            ti.dispatchEvent(new KeyboardEvent('keypress',{'key':13}));*/
            //console.log(evt.keyCode);
        }
    });
    //var tcont = document.getElementsByClassName('terminal-container')[0];
    //tcont.style.background="blue";

    // var ttype = document.getElementsByClassName('terminal-type')[0];
}

window.addEventListener("load", do_load);
if (typeof app_title === "function") {
    document.addEventListener("keyup", exit_on_esc);
}