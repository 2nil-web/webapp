#!/bin/bash

color(){
    for c; do
        printf ' \e[48;5;%dm%03d ' $c $c
    done
    printf '\e[0m \n'
}

IFS=$' \t\n'

if true
then
echo -e "\ncolor {0..15}"
color {0..15}

echo -e "\ncolor {16..231}"
for ((i=0;i<6;i++)); do
    color $(seq $((i*36+16)) $((i*36+51)))
done

echo -e "\ncolor {232..255}"
color {232..255}
else
color {0..255}
fi

