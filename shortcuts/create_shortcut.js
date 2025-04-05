
var activeX=[];
function callActiveX(AXName, add) {
  if (typeof activeX[AXName] === 'undefined') activeX[AXName]=new ActiveXObject(AXName);
  return activeX[AXName];
}

function fso() { return callActiveX("Scripting.FileSystemObject"); }
function wsh() { return callActiveX("WScript.Shell"); }
function sha() { return callActiveX("Shell.Application"); }
function wsn() { return callActiveX('Wscript.Network'); }
function wsn() { return callActiveX('ScriptControl'); }

/* style: * 0: HIDE * 1: NORMAL * 2: MINIMIZED * 3: MAXIMIZED */

function shortcut (name, targ, args, wdir, icon, desc, styl, hkey) {
  this.set = function (name, targ, args, wdir, icon, desc, styl, hkey) {
    this.name=typeof name !== "undefined" ? name : "";
    this.targ=typeof targ !== "undefined" ? targ : "";
    this.args=typeof args !== "undefined" ? args : "";
    this.wdir=typeof wdir !== "undefined" ? wdir : "";
    this.icon=typeof icon !== "undefined" ? icon : "";
    this.desc=typeof desc !== "undefined" ? desc : "";
    this.styl=typeof styl !== "undefined" ? styl : 1;
    this.hkey=typeof hkey !== "undefined" ? hkey : "";
  }

  this.info = function() {
    msg ="FullName         : "+this.lnk.FullName+"\n";
    if (this.lnk.Arguments !== "") msg+="Arguments        : "+this.lnk.Arguments+"\n";
    msg+="TargetPath       : "+this.lnk.TargetPath+"\n";
    if (this.lnk.WorkingDirectory !== "") msg+="WorkingDirectory : "+this.lnk.WorkingDirectory+"\n";
    msg+="WindowStyle      : "+this.lnk.WindowStyle+"\n";
    if (this.lnk.Description !== "") msg+="description      : "+this.lnk.Description+"\n";
    if (this.lnk.Hotkey !== "") msg+="Hotkey           : "+this.lnk.Hotkey+"\n";
    if (this.lnk.IconLocation !== "") msg+="iconlocation     : "+this.lnk.IconLocation+"\n\n";
    return msg;
  }

  this.save = function() {
    this.lnk.Save();
    return this.info();
  }


  // Create shortcut if name and targ are not empty and return a string with info creation
  // Else return ""
  this.create = function() {
    if (this.name !== "" && this.targ !== "") {
      this.lnk=wsh().CreateShortcut(this.name+".lnk");
      this.lnk.TargetPath=this.targ;
      this.lnk.Arguments=this.args;
      this.lnk.WorkingDirectory=this.wdir;
      this.lnk.WindowStyle=this.styl;
      if (this.desc !== "") this.lnk.Description=this.desc;
      if (this.icon !== "") this.lnk.IconLocation=this.icon;
      if (this.hkey !== "") this.lnk.Hotkey=this.hkey; // "CTRL+SHIFT+F"
      return true;
    }

    return false;
  }

  this.recreate = function(name, targ, args, wdir, icon, desc, styl, hkey) {
    this.set(name, targ, args, wdir, icon, desc, styl, hkey);

    if (this.create()) {
      this.save();
      return this.info();
    }
    return "";
  }

  this.recreate(name, targ, args, wdir, icon, desc, styl, hkey);
}

