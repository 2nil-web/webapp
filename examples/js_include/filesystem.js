const file_type = {
  not_found: 0,
  none: 1,
  regular: 2,
  directory: 3,
  symlink: 4,
  block: 5,
  character: 6,
  fifo: 7,
  socket: 8,
  unknown: 9
};
// Return file type in a string form
function ftype2s(t) {
  switch (t) {
    case file_type.none:
      return "`not-evaluated-yet`";
    case file_type.not_found:
      return "not found";
    case file_type.regular:
      return "regular file";
    case file_type.directory:
      return "directory";
    case file_type.symlink:
      return "symlink";
    case file_type.block:
      return "block device";
    case file_type.character:
      return "character device";
    case file_type.fifo:
      return "named IPC pipe";
    case file_type.socket:
      return "named IPC socket";
    case file_type.unknown:
      return "`unknown`";
    default:
      return "`implementation-defined`";
  }
}
const perms = {
  none: 0,
  others_exec: 01,
  others_write: 02,
  others_read: 04,
  others_all: 07,
  group_exec: 010,
  group_write: 020,
  group_read: 040,
  group_all: 070,
  owner_exec: 0100,
  owner_write: 0200,
  owner_read: 0400,
  owner_all: 0700,
  all: 0777,
  sticky_bit: 01000,
  set_gid: 02000,
  set_uid: 04000,
  mask: 07777,
  unknown: 0xFFFF
};
// Return file permissions in a string form
function perm2s(p) {
  sp = "";
  if (p === perms.unknown) sp += "unknown,";
  else {
    if (p & perms.all) sp += "all,";
    else {
      if (p & perms.none) sp += "none,";
      if (p & perms.others_all) sp += "others_all,";
      else {
        if (p & perms.others_exec) sp += "others_exec,";
        if (p & perms.others_write) sp += "others_write,";
        if (p & perms.others_read) sp += "others_read,";
      }
      if (p & perms.group_all) sp += "group_all,";
      else {
        if (p & perms.group_exec) sp += "group_exec,";
        if (p & perms.group_write) sp += "group_write,";
        if (p & perms.group_read) sp += "group_read,";
      }
      if (p & perms.owner_all) sp += "owner_all,";
      else {
        if (p & perms.owner_exec) sp += "owner_exec,";
        if (p & perms.owner_write) sp += "owner_write,";
        if (p & perms.owner_read) sp += "owner_read,";
      }
    }
    if (p & perms.mask) sp += "mask,";
    else {
      if (p & perms.sticky_bit) sp += "sticky_bit,";
      if (p & perms.set_gid) sp += "set_gid,";
      if (p & perms.set_uid) sp += "set_uid,";
    }
  }
  // Remove last comma
  sp = sp.slice(0, -1)
  return sp;
}
// Return file permissions in the form 'drwxrwxrwx'
function show_status(fsta) {
  function prm(op, p) {
    return fsta.perms && p ? op : '-';
  }
  var ret = "";
  if (fsta.type == file_type.directory) ret = 'd';
  else ret = '-';
  ret += prm('r', perms.owner_read);
  ret += prm('w', perms.owner_write);
  ret += prm('x', perms.owner_exec);
  ret += prm('r', perms.group_read);
  ret += prm('w', perms.group_write);
  ret += prm('x', perms.group_exec);
  ret += prm('r', perms.others_read);
  ret += prm('w', perms.others_write);
  ret += prm('x', perms.others_exec);
  //console.log(JSON.stringify(fsta)+"="+fsta.perms+"<=>"+ret+"<=>"+perm2s(fsta.perms));
  return ret;
}
// Chemins Windows ou Unix
function is_unc_path(s) {
  return /^\\\\[^\\\/]+\\[^\\\/]+(\\[^\\\/]*)*$/.test(s);
}

function is_win_path(s) {
  return /^[a-zA-Z]:/.test(s) || (/^[a-zA-Z]:\\/.test(s) && /\\/.test(s));
}

function is_unc_or_win_path(s) {
  if (is_unc_path(s) || is_win_path(s)) return true;
  return false;
}

function is_absolute_win_path(s) {
  return /^[a-zA-Z]:\\/.test(s) || /^\\\\/.test(s);
}

function is_relative_win_path(s) {
  return !is_absolute_win_path(s);
}
// Convert a path to windows path by replacing ^/X/ to X:\\ and any other / by \\
function to_win_path(s) {
  if (is_unc_or_win_path(s)) return s;
  return s.replace(/^\/(.)\//, "$1:\\").replace(/\//g, "\\");
}

function is_unix_path(s) {
  return /^(\/[a-zA-Z0-9._-]+)*\/?$/.test(s);
}

function is_absolute_unix_path(s) {
  return s.startsWith('/');
}

function is_relative_unix_path(s) {
  return !s.startsWith('/');
}
// Convert a path to unix path by replacing ^X:\\ by /X/ and replacing any other \\ by /
function to_unix_path(s) {
  if (is_unix_path()) return s;
  if (is_absolute_win_path) return '/' + s.replace(/\\/g, "/").replace(/:/, "");
  return s.replace(/\\/g, "/").replace(/:/, "");
}
if (false) {
  function log_path(s) {
    res = s + " is ";
    if (is_unix_path(s)) {
      console.log("Unix path => [" + s + "] <=> [" + to_win_path(s) + "] <= Windows path");
    }
    if (is_unc_or_win_path(s)) {
      console.log("Windows UNC path [" + s + "] <=> [" + to_unix_path(s) + "] <= Unxi path");
    }
  }
  log_path("Chemin_RelatifWindows\\Fichier.txt");
  log_path("Chemin_RelatifUnix/Fichier.txt");
  log_path("\\\\Serveur\\Partage\\Chemin\\Fichier.txt");
  log_path("C:\\Users\\NomUtilisateur\\Documents\\fichier.txt");
  log_path("/usr/local/bin");
  log_path("\\\\Serveur\\Dossier");
  log_path("C:");
  log_path("C:\\toto\\");
  log_path("\\toto\\");
  log_path("/C/toto");
  log_path("/usr/local/bin");
  log_path("/home/user/Documents/file.txt");
  log_path("C:\\Users\\NomUtilisateur\\Documents\\fichier.txt");
  log_path("\\\\Serveur\\Partage\\Fichier");
}