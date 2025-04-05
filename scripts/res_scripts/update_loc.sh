#!/bin/bash

if [ "$HOSTNAME" = "FRTLS-2765" ]
then
  find . -name "*.exe" | sed 's/^\.\///' | while read ln
  do
    if [ ! -f "/p/webview-app/$ln" ] || [ "$ln" -nt "/p/webview-app/$ln" ]
    then
      echo "Copying $ln to /p/webview-app/$ln"
      cp -f "$ln" "/p/webview-app/$ln"
    fi
  done
fi

