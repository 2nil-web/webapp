let OSName = client_system();
var is_windows = false;
if (OSName === "Windows") is_windows = true;

var appName = "JSTerm";
// Fonctionnement spécifique a webview-app
if (typeof app_set_title === "function") {
    appName = app_set_title(document.title);
    // Prevent the app to load multiple times with the help of a BroadcastChannel

    // Eventually clear all localStorage values
    // localStorage.clear();
    // Eventually display the configuration data to the console
    //for (const key of Object.keys(localStorage)) { println("onload: "+key+": "+localStorage.getItem(key)); }

    // Set window title, icon and minimal dimensions
    app_set_title(appName);
    app_set_icon("app.ico");
    app_set_size(200, 80, 1);
}

function exit_on_esc() {
    if (event.keyCode === 27) app_exit();
}


function scrollToBottom() {
    output.scrollTop = output.scrollHeight;
}

function cls() {
    output.innerHTML = "";
    output.scrollTop = output.scrollHeight;
}

function print(s, scrollToBottom = true) {
    output.innerHTML += s;
    if (scrollToBottom) output.scrollTop = output.scrollHeight;
}

function println(s, scrollToBottom = true) {
    output.innerHTML += s + "<br/>";
    if (scrollToBottom) output.scrollTop = output.scrollHeight;
}

function fl_sort(a, b) {
    if (a.startsWith('*')) a1 = a.substring(1);
    else a1 = a;
    if (b.startsWith('*')) b1 = b.substring(1);
    else b1 = b;
    return a1 > b1;
}

var func_list = [],
    func_help = [];
(async () => {
    if (func_list.length === 0) {
        hjs = await window.app_help("json");

        Object.keys(hjs).forEach((k) => {
            func_list.push(k + "() // " + hjs[k]);
            func_help.push(k + ": " + hjs[k]);
        });

        func_list.sort(fl_sort);
        func_help.sort(fl_sort);
    }
})();