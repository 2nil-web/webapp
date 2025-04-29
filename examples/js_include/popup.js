// ### Win32 and COM API ###
var activeX = [];

function callActiveX(AXName) {
  if (typeof activeX[AXName] === 'undefined') activeX[AXName] = new ActiveXObject(AXName);
  return activeX[AXName];
}

function fso() {
  return callActiveX("Scripting.FileSystemObject");
}

function wsh() {
  return callActiveX("WScript.Shell");
}

function sha() {
  return callActiveX("Shell.Application");
}

function wsn() {
  return callActiveX('Wscript.Network');
}

function scc() {
  return callActiveX('ScriptControl');
}
var MB = {
  Ok: 0,
  OkCancel: 1,
  AbortRetryIgnore: 2,
  YesNoCancel: 3,
  YesNo: 4,
  RetryCancel: 5,
  CancelTryAgainContinue: 6,
  Stop: 16,
  Question: 32,
  Exclamation: 48,
  Information: 64,
  Default2ndButton: 256,
  Default3rdButton: 512,
  Modal: 4096,
  RightAlignText: 524288,
  RightToLeftText: 1048576
};
var ID = {
  Ok: 1,
  Cancel: 2,
  Abort: 3,
  Retry: 4,
  Ignore: 5,
  Yes: 6,
  No: 7,
  TryAgain: 10,
  Continue: 11,
  TimeOut: -1
};

function msj_alert(s) {
  wsh().Popup(s, 0, WScript.ScriptName, MB.Ok + MB.Information);
}

function msj_confirm(s) {
  rep = wsh().Popup(s, 0, WScript.ScriptName, MB.OkCancel + MB.Question);
  return (rep == ID.Ok);
}
msg = "Message";
nSecondsToWait = 0;
title = "Title";
nType = MB.OkCancel + MB.Question;
wsh().Popup(msg, nSecondsToWait, title, nType);