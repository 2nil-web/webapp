
SHELL=bash

ifndef MAKEFLAGS
##OBJS:=$(addprefix ${TARGET_DIR}/,${OBJS})
CPUS ?= $(shell nproc)
MAKEFLAGS += -j $(CPUS) # -l $(CPUS) # --no-builtin-rules -s
endif

VERSION=$(shell git describe --abbrev=0 --tags 2>/dev/null || echo 'Unknown_version')
COPYRIGHT=(C) D. LALANNE - MIT License.
DECORATION=
COMMIT=$(shell git rev-parse --short HEAD 2>/dev/null || echo 'Unknown_commit')
ISO8601 := $(shell date +%Y-%m-%dT%H:%M:%SZ)
TMSTAMP := $(shell date +%Y%m%d%H%M%S)

# Génération du version.h intégré dans l'appli
src/version.h : version_check.txt
	@${ECHO} "Building C++ header $@"
#	${ECHO} "#ifndef VERSION_H\n#define VERSION_H\nstruct s_app_info\n{\n  std::string name = \"${PREFIX}\", version = \"${VERSION}\", copyright = \"${COPYRIGHT}\", decoration = \"${DECORATION}\", commit = \"${COMMIT}\", created_at = \"${ISO8601}\";\n};\nstatic s_app_info app_info;\n#endif" >$@
	${ECHO} "#ifndef VERSION_H\n#define VERSION_H\nstruct\n{\n  std::string name, version, copyright, decoration, commit, created_at;\n} app_info = {\"${PREFIX}\", \"${VERSION}\", \"${COPYRIGHT}\", \"${DECORATION}\", \"${COMMIT}\", \"${ISO8601}\"};\n#endif" >$@
	dos2unix $@

# Génération du version.json intégré dans le paquetage
version.json : version_check.txt
	@${ECHO} "Building json file $@"
	@${ECHO} -e '{ "name":"${PREFIX}", "version":"${VERSION}", copyright:"${COPYRIGHT}", "decoration":"${DECORATION}", "commit":"${COMMIT}","created_at":"${ISO8601}" }' >$@
	dos2unix $@

# Pour regénérer silencieusement version.h et version.json dès qu'un des champs version ou copyright ou decoration ou commit, est modifié.
version_check.txt : FORCE
	@${ECHO} "Version:${VERSION}, copyright:${COPYRIGHT}, decoration:${DECORATION}, commit:${COMMIT}" >new_$@
	@-( if [ ! -f $@ ]; then cp new_$@ $@; sleep 0.4; fi )
	@-( if diff new_$@ $@ >/dev/null 2>&1; then rm -f new_$@; \
		  else mv -f new_$@ $@; rm -f ${PREFIX}.iss ${PREFIX}-standalone.iss; fi )

ignupd :
	@$(call igup,)

cfg :
	@echo "DO_MSBUILD: ${DO_MSBUILD}"
	@echo "MSBUILD: ${MSBUILD}"
	@echo "GCC: ${GCC}"
	@echo "CC: ${CC}"
	@echo "CXX: ${CXX}"
	@echo "CXXFLAGS: ${CXXFLAGS}"
	@echo "CPPFLAGS: ${CPPFLAGS}"
	@echo "OBJS: ${OBJS}"
	@echo "LDFLAGS: ${LDFLAGS}"
	@echo "OS_ID: ${OS_ID}"
	@echo "SYS_VER: ${SYS_VER}"
	@echo "GTK_WEBKIT: ${GTK_WEBKIT}"
	@echo "TARGET_DIR: ${TARGET_DIR}"
	@echo "TARGET_DIR_CONS: ${TARGET_DIR_CONS}"
	@echo "TARGET_API: ${TARGET_API}"
	@echo "TARGET: ${TARGET}"
	@echo "TARGET_CONS: ${TARGET_CONS}"
	@echo "TARGETS: ${TARGETS}"
	@echo "BUILD_SYS: ${BUILD_SYS}"
#	@echo "lastword BUILD_SYS: $(lastword $(BUILD_SYS))"
#	@echo "patsubst BUILD_SYS: $(patsubst %_gcc,gcc,$(BUILD_SYS))"
	@echo "SRCS: ${SRCS}"
	@echo "RES_SRC: ${RES_SRC}"
	@echo "OBJS: ${OBJS}"
ifneq ($(EXES),)
	@echo "TARGET: ${EXES}"
endif
ifneq ($(TARGET),)
	@echo "TARGET: ${TARGET}"
endif
	@echo "OS: ${OS}"
	@echo "PGF: ${PGF}"
	@echo "PGF86: ${PGF86}"
	@echo "PGU: ${PGU}"
ifneq (${DISP_PATH},)
	@echo "PATH"
	@echo "${PATH}" | sed 's/:/\n/g'
	@echo "END PATH"
endif
	@echo "DO_MSBUILD [${DO_MSBUILD}]"
ifeq (${DO_MSBUILD},yes)
	@echo "MSBUILD [${MSBUILD}]"
endif
	@echo "TMP: ${TMP}"
	@echo "TEMP: ${TEMP}"
	@echo "tmp: ${tmp}"
	@echo "temp: ${temp}"
	@echo "root_dir: ${root_dir}"
	@echo "mkfile_path: ${mkfile_path}"
	@echo "mkfile_dir: ${mkfile_dir}"
	@echo "current_dir: ${current_dir}"

help :
	@echo -e "$(shell tput bold; tput smul)What to do to build and/or deliver a new version of ${PREFIX} ?$(shell tput sgr0)"
	@echo "  I - $(shell tput bold)Under Windows the app may be compiled with gcc and/or Visual Studio.$(shell tput sgr0)"
	@echo -e "    1. Under the Windows explorer, double-click on the 'webapp.sln' file, if you have Visual Studio already installed."
	@echo -e "    2. Under MSYS2 bash, by simply running the 'make' command and if Visual Studio is set according to the content of the MSBUILD variable(see header.mk) then it will be the default compiler."
	@echo -e "    3. If you have a correctly set Visual Studio configuration but want to force the use of gcc, then:"
	@echo -e "      a) Type 'make gcc' to build the binaries with gcc."
	@echo -e "      b) Or set the bash env variable DO_MSBUILD to 'no' (export DO_MSBUILD=no) for a complete switch, up to the delivery."
	@echo -e " II - $(shell tput bold)On any other build system (Linux) gcc is the default.$(shell tput sgr0)"
	@echo -e "III - $(shell tput bold)Prepare build and delivery$(shell tput sgr0)"
	@echo -e "$(shell tput sitm)  If, for instance, you are in a develop branch and want to merge its last commit to master, then do the following:$(shell tput sgr0)"
	@echo -e "    1-Display commit  : git log -1 --oneline # or git show --oneline -s"
	@echo -e "    2-Set a variable  : ck=\044(git log -1 --oneline | sed 's/ .*//')"
	@echo -e "    3-Go to master    : git checkout master"
	@echo -e "    4-Retrieve commit : git cherry-pick \044ck"
	@echo -e "    5-Back to develop : git checkout develop # To go on with your developmements, if needed."
	@echo -e " IV - $(shell tput bold)Build and delivery steps$(shell tput sgr0)"
	@echo -e "    1-Check remote tags : git ls-remote --tags origin"
	@echo -e "    2-Check local tags  : git describe --abbrev=0 --tags"
	@echo -e "    3-New version tag   : git tag -a X.Y.Z-nom_de_la_prerelease -m 'commentaire' # De préférence un tag annoté (-a)."
	@echo -e "    4-Push a tag        : git push --tags"
	@echo -e "    5-Build application : make ... (the only mandatory step before delivery)"
	@echo -e "    6-Build the setup   : make setup # (Only a zip creation for now, could be improved by creating a 'real' Windows and/or Linux setup)"
	@echo -e "    7-Delivery          : make deliv (may be run directly after 'make ...' step."
	@echo -e "  V - $(shell tput bold)For versioning$(shell tput sgr0)"
	@echo -e "    Respect Semantic Versioning in the delivery tags (see https://semver.org)."
	@echo -e "    A short example according to the pattern 'MAJOR.MINOR.PATCH-pre_release+metadata' might be something like : 0.1.1-pre_alpha_release+Latest.bug.fixes.on.sha.965c58e"
	@echo -e "  VI - $(shell tput bold)From time to time$(shell tput sgr0)"
	@echo -e "     Run: 'scripts/update_webviews.sh' to check the versions of webview and eventually MS Webview2."
	@echo -e " VII - $(shell tput bold)For more details$(shell tput sgr0)"
	@echo -e "     Have a look at the following files : header.mk, Makefile, rules.mk, webapp.vcxproj and webapp-console.vcxproj."


# Ces régles implicites ne sont pas utiles quand on fait 'make clean'
ifneq ($(MAKECMDGOALS),clean)
ifeq ($(DO_MSBUILD),yes)
define genmsexe
	if [ ${1} != TemplateApp ]; \
	then \
		sed 's/\<TemplateApp\>/$(1)/g' TemplateApp.sln >${1}.sln; \
		sed 's/\<TemplateApp\.cpp\>/$(1).cpp/g;s/\<TemplateApp\>/$(1)/g' TemplateApp.vcxproj >${1}.vcxproj; \
	fi; \
		${MSBUILD} ${1}.sln -p:Configuration=${CONF} >${1}.mslog && rm -f ${1}.mslog; 
	if [ ${1} != TemplateApp ]; \
	then \
		[ -f ${TARGET_DIR}/${1}.exe ] && ( rm -f ${1}.sln ${1}.vcxproj* ); \
	fi
endef
endif

ifneq ($(EXES),)
	TGT=${EXES}
endif
ifneq ($(TARGET),)
	TGT=${TARGET}
endif
define igup
	echo ${TGT}
	$(shell ( echo ${TGT} | grep ^TARGET | sed 's/^TARGET //;s/ /\n/g' | while read tgt; do echo $tgt; done ) >toto)
endef

%.html : %.md
	pandoc -o $@ --pdf-engine=xelatex $<

%.docx : %.html
	pandoc -o $@ -f html -t docx $<

%.pdf : %.docx
	pandoc -o $@ -f docx -t pdf $<

#%.pdf : %.html
#	pandoc -o $@ --pdf-engine=xelatex $< | true

%.pdf : %.md
	pandoc -o $@ --pdf-engine=xelatex $< | true

# Multiple resolution or not ?
${SRC_DIR}/%.ico : %.png
	${MAGICK} $< -density 300 -define icon:auto-resize=128,96,64,48,32,16 -background none $@

#${SRC_DIR}/%.ico : %.svg
#	${MAGICK} -density 256x256 -background none $< -define icon:auto-resize=128,96,64,48,32,16 -colors 256 $@
#	${MAGICK} $< -density 300 -define icon:auto-resize=128,96,64,48,32,16 -background none $@

%.ico : %.png
	${MAGICK} -density 256x256 -background none $< -define icon:auto-resize=128,96,64,48,32,16 -colors 256 $@
#	${MAGICK} $< -density 300 -define icon:auto-resize=128,96,64,48,32,16 -background none $@

%.png : %.svg
	inkscape --export-type=png --export-background-opacity=0 $<
#	${MAGICK} $< -background none $@

ifneq ($(MAKECMDGOALS),clean)
ifndef NO_OBJS
${TARGET_DIR}/%.o: %.c
	@mkdir -p ${TARGET_DIR}
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

${TARGET_DIR}/%.o: ${SRC_DIR}/%.cpp
	@mkdir -p ${TARGET_DIR}
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

${TARGET_DIR}/%${EXEXT}: ${TARGET_DIR}/%.o
	@mkdir -p ${TARGET_DIR}
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@
endif

${TARGET_DIR}/%: ${TARGET_DIR}/%.o
	@mkdir -p ${TARGET_DIR}
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

${TARGET_DIR}/%${EXEXT}: %.c
	@mkdir -p ${TARGET_DIR}
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@
endif

${TARGET_DIR}/%${EXEXT}: %.cpp
ifneq ($(DO_MSBUILD),no)
	@echo "Building $@ from $^ with msvc"
	@$(call genmsexe,$*)
else
	@echo "Building $@ from $^ with g++"
	@mkdir -p ${TARGET_DIR}
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@
endif

ifeq ($(DO_MSBUILD),no)
# Régles pour construire les fichier objet d'après les .rc
${TARGET_DIR}/%.o : ${SRC_DIR}/%.rc
	@mkdir -p ${TARGET_DIR}
	$(RC) --use-temp-file $< --include-dir . $(OUTPUT_OPTION)
# Have to use option '--use-temp-file' because of issue with popen ...

${TARGET_DIR}/%.d: %.c
	@echo Checking header dependencies from $<
	@mkdir -p ${TARGET_DIR}
	@echo -n "${TARGET_DIR}/" > $@
	@($(COMPILE.c) -isystem /usr/include -MM $< >> $@

#	@echo "Building "$@" from "$<
${TARGET_DIR}/%.d: ${SRC_DIR}/%.cpp
	@echo Checking header dependencies from $<
	@mkdir -p ${TARGET_DIR}
	@echo -n "${TARGET_DIR}/" > $@
	$(COMPILE.cpp) -isystem /usr/include -MM $< >> $@

# Inclusion des fichiers de dépendance .d
ifdef OBJS
-include $(OBJS:.o=.d)
endif
endif
endif

