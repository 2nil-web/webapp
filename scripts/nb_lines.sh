#!/bin/bash

echo "Nb lines in"
echo -n "  webview: "
( cd webview && wc -l $(find cmake/ core/ docs/ examples/ packaging/ scripts/ test_driver/ -type f) ) | tail -1 | sed 's/ *\([0-9]*\).*/\1/'


echo -n "  webapp:  "
cat $(file *|grep -i text|sed 's/:.*//') $(\ls -1 src/* | grep -v src/wa-winerrv.h) scripts/*.sh shortcuts/* examples/*/*.{js,html,css} | wc -l

