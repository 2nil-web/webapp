function is_webview() {
  if (typeof app_icon === "function") {
    //console.log("IS WEBVIEW");
    return true;
  } else return false;
}

function not_app_func() {
  console.log("Not a webapp function " + not_app_func.caller.name);
}
/*
 Reste à ajouter
  app_exec
  app_get_title
  app_get_title_bg
  app_hide
  app_minimize
  app_on_exit_msg
*/
var webapp = {
  title: function(s) {
    if (typeof app_set_title === "function") app_set_title(s);
    else not_app_func();
  },
  icon: function(ic) {
    if (typeof app_set_icon === "function") app_set_icon(ic);
    else not_app_func();
  },
  restore: function() {
    if (typeof app_restore === "function") app_restore();
    else not_app_func();
  },
  size: function(w, h, hint = 0) {
    if (typeof app_set_size === "function") app_set_size(w, h, hint);
    else not_app_func();
  },
  pos: function(x, y) {
    if (typeof app_set_pos === "function") app_set_pos(x, y);
    else not_app_func();
  },
  on_move: function(func) {
    if (typeof app_on_move === "function") app_on_move(func);
    else not_app_func();
  },
  hints: function(hnts) {
    /* 0 Width and height are default size
       1 Width and height are minimum bounds
       2 Width and height are maximum bounds
       3 Window size is fixed */
    if (typeof app_hints === "function") app_hints(hnts);
    else not_app_func();
  },
  getenv: function(s) {
    if (typeof getenv === "function") return getenv(s);
    not_app_func();
    return "";
  },
  shell: function(s) {
    if (typeof app_shell === "function") return app_shell(s);
    not_app_func();
    return "";
  },
  exit: function() {
    if (typeof app_exit === "function") app_exit();
    else not_app_func();
  },
};