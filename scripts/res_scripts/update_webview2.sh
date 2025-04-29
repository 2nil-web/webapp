#!/bin/bash

function BoldEcho () {
  echo
  echo "$(tput smso)$*$(tput rmso)"
}

WVDir=webview

if [ ! -d ${WVDir} ]
then
  BoldEcho "Downloading latest version of ${WVDir}"
  git clone https://github.com/webview/webview.git
fi

PrevWV2Dir=${WVDir}/build/external/libs
NextWV2Dir=${PrevWV2Dir}/NewWV2

BoldEcho "Downloading latest version of nuget"
curl -ksSLO https://dist.nuget.org/win-x86-commandline/latest/nuget.exe

BoldEcho "Downloading latest version of Webview2"
rm -rf ${NextWV2Dir}
./nuget.exe install Microsoft.Web.Webview2 -OutputDirectory ${NextWV2Dir}

PrevWebView2=$(sed -n 's/.*\(Microsoft\.Web\.WebView2\..*\)\\build.*/\1/p' webview-app.vcxproj | head -1)
NextWebView2=$(cd ${NextWV2Dir} && \ls -1d Microsoft.Web.WebView2.* | sort | tail -1)

BoldEcho "PrevWebView2=${PrevWebView2}"
BoldEcho "NewWebView2=${NewWebView2}"

if [ "$PrevWebView2" = "$NextWebView2" ]
then
  BoldEcho "Already with latest version of WebView2"
  #rm -rf ${NextWV2Dir}
else
  BoldEcho "Updating to newer version of WebView2 for msvc and gcc"
  mkdir -p ${PrevWV2Dir}/OldWV2
  [ -d "${PrevWV2Dir}/${PrevWebView2}" ] && mv "${PrevWV2Dir}/${PrevWebView2}" "${PrevWV2Dir}/OldWV2"
  mv "${NextWV2Dir}/${NextWebView2}" "${PrevWV2Dir}"
# Update for MSBuild
  sed -i "s/${PrevWebView2}/${NextWebView2}/" webview-app.vcxproj
# Update for Makefile
  sed -i "s/^WV2SUBDIR=.*/WV2SUBDIR=${NextWebView2}/" header.mk
fi

