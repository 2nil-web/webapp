
SRC_DIR=.
include ../../header.mk

PATH:=${PATH}:/c/UnixTools/bin:~/.cargo/bin/tokei

TOKEI=tokei

CMD=cd ../.. && ${TOKEI} --hidden --exclude winerrv.h Makefile *.mk src scripts/*.sh assets shortcuts examples/command_line examples/js_include examples/backup examples/webcrypto examples/stats

all : app.ico app.png app_dark.ico app_dark.png
	 @( ${CMD} ) | sed 's/ Header/_Header/;s/ Text/_Text/' >stats.txt
	 @cat stats.txt
	 @( cat stats.txt | sed "1d;3d;$d;/---/d;/===/d;;s/|- /|-/;s/(Total)/&;/;s/  */;/g;s/^;//" )>stats.csv
	 @( ${CMD} -o json | jq ) >stats.json

clean :
	rm -f stats.json stats.csv stats.txt app.ico app.png app_dark.ico app_dark.png

include ../../rules.mk

