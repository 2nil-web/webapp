///////////// TEST récup X,Y, W, H avant exit

//localStorage.clear(); // Clear all localStorage values
//for (const key of Object.keys(localStorage)) { console.log("onload "+key, localStorage.getItem(key)); } ; console.log("");

function getItemOrDefault(itemId, defVal, msg = "") {
    itemVal = localStorage.getItem(itemId);

    if (itemVal === null || itemVal === "") {
        itemVal = defVal;
        //    console.log(`For item ${itemId} default ${msg}value is '${itemVal}'`);
        localStorage.setItem(itemId, itemVal);
    } else {
        //    console.log(`For item ${itemId} localStorage ${msg}value is '${itemVal}'`);
    }

    return itemVal;
};

function getBoolItemOrDefault(itemId, defVal) {
    return (getItemOrDefault(itemId, defVal, "boolean ") === "true");
};

function save_pos() {
    webapp_get_pos().then(pos => {
        localStorage.setItem("webapp.x", pos.x);
        localStorage.setItem("webapp.y", pos.y);
        localStorage.setItem("tpb_url", tpb_url.value);
        console.log(`SAVE POS ${pos.x}, ${pos.y}`);
    });
}

function save_dim() {
    localStorage.setItem("window.outerWidth", window.outerWidth);
    localStorage.setItem("window.outerHeight", window.outerHeight);
    console.log(`SAVE DIM ${window.outerWidth}, ${window.outerHeight}`);
}

function getwinrec() {
    webapp_get_pos().then(pos => {
        alert(`${pos.x}, ${pos.y}, ${window.outerWidth}, ${window.outerHeight}`);
        console.log(`${pos.x}, ${pos.y}, ${window.outerWidth}, ${window.outerHeight}`);
    });
}

function save_pos_and_exit() {
    save_pos();
    webapp_exit();
}

var winX, winY, winW, winH;


function conf_load() {
    winX = getItemOrDefault("webapp.x", 640);
    winY = getItemOrDefault("webapp.y", 390); //-3;
    //  console.log(`GET POS ${winX}, ${winY}`);
    winW = getItemOrDefault("window.outerWidth", 640);
    winH = getItemOrDefault("window.outerHeight", 360); //-3;
    //  console.log(`GET DIM ${window.outerWidth}, ${window.outerHeight}`);
}
///////////// FIN TEST récup X,Y avant exit

conf_load();

if (typeof webapp_get_title === "function") {
    // Default and minimal size
    webapp_set_title("TPB browse");
    webapp_show();
    webapp_set_size(winW, winH);
    webapp_set_pos(winX, winY);
    //  webapp_hints(3);
    webapp_on_move('save_pos()');
    //webapp_on_exit('save_pos()');
}

document.addEventListener("keyup", (event) => {
    if (event.keyCode === 27) {
        save_pos_and_exit();
    }
});

window.addEventListener("keyup", (event) => {
    if (event.keyCode === 27) {
        save_pos_and_exit();
    }
});

window.addEventListener("resize", (event) => {
    save_dim();
});


// Polyfills
String.prototype.insert = function(str, pos) {
    return this.substring(0, pos) + str + this.substring(pos)
};

function about() {
    info = "";
    app_info().then((ainf) => {
        console.log("app info=" + ainf);
        info = ainf.replace(/,/g, "\n") + "\nIncludes\n";

        app_info().then((wvver) => {
            info += " WebView version: " + wvver + "\n";
            console.log("app_info=" + wvver);

            // May not include libcurl
            if (typeof libcurl_ver !== "undefined") {
                libcurl_ver().then((lcver) => {
                    info += " LibCurl version: " + lcver;
                    console.log("libcurl_ver=" + lcver);
                    window.alert(info);
                });
            } else window.alert(info);
        });
    });

}


function decodeEntities(html) {
    var txt = document.createElement("textarea");
    txt.innerHTML = html;
    return txt.value;
}

async function help(obj = output, fmt = "") {
    window.webapp_help(fmt).then(help_msg => {
        if (fmt === "json") {
            help_msg = decodeEntities(JSON.stringify(help_msg, null, 0).replace(/[{}"]/g, "").replace(/,/g, "\n"));
        } else {
            help_msg = decodeEntities(help_msg);
        }
        if (obj instanceof HTMLElement) obj.value += help_msg;
        console.log(help_msg);
    });
}


function get_delim(str, delim1, delim2) {
    start = str.indexOf(delim1);

    if (start > -1) {
        str = str.substring(start);
        end = str.indexOf(delim2);
        if (end > -1) str = str.substring(end);
    }
    return [start, start + end, str];
}

function get_delim_substr(str, delim1, delim2) {
    [start, end, rem] = get_delim(str, delim1, delim2);
    if (start > -1 && end > -1) {
        return [str.substring(start, end + delim2.length), rem];
    }
    //console.log(`start:${start}-delim1:${delim1}, end:${end}-delim2:${delim2}`);
    return [null, null];
}

function advance_before_delim(str, delim) {
    p = str.indexOf(delim);
    if (p === -1) p = 0;
    return str.substring(p);
}

function advance_after_delim(str, delim) {
    return advance_before_delim(str, delim).substring(delim.length);
}

var trc = false;

function filt_pron(orig_txt) {
    var res = "";
    [res, txt] = get_delim_substr(orig_txt, '<head>', '</thead>');

    var item = "";
    while (true) {
        [item, txt] = get_delim_substr(txt, '<tr>', '<\/tr>');
        if (item === null) break;
        if (item.includes('title="More from this category">Video &gt')) {
            res += item;
        }
    }

    return res;
}


async function recent_tpb_fetch(onEnterOnly) {
    url = tpb_url.value;
    if (!url.endsWith('/')) url += '/';
    localStorage.setItem("tpb_url", url);
    recent_url = url + "recent/";

    list.innerHTML = "";

    var ht = "";
    var i = 1;
    while (true) {
        const response = await fetch(recent_url + i, {
            mode: "cors"
        });
        if (!response.ok) break;
        ht += await response.text();
        i++;
    }

    //  console.log(ht);

    ht = filt_pron(ht);
    ht = ht.replace("max-width: 1000px", "max-width: 95%");
    ht = ht.replace(/href=\"\//g, 'href="' + url);

    list.innerHTML = ht + "</body></html>";
}

// Send magnet to transmission : https://gist.github.com/sbisbee/8215353

window.addEventListener("load", (event) => {
    tpb_url.value = getItemOrDefault("tpb_url", "https://thepiratebay10.info");
    document.getElementById('tpb_url').addEventListener("keydown", function(e) {
        if (e.keyCode == 13) {
            recent_tpb_fetch();
        }
    });
    recent_tpb_fetch();
});