
SRC_DIR=src
ASSETS=assets

include header.mk

PREFIX=webapp
SRCS=$(wildcard ${SRC_DIR}/*.cpp)
#SRCS:=$(filter-out ${SRC_DIR}/getopt.cpp,${SRCS})
#SRCS:=$(addprefix ${SRC_DIR}/,${SRCS})

ifeq (${TARGET_API},windows)
RES_SRC=${SRC_DIR}/${PREFIX}_res.rc
else
SRCS:=$(filter-out ${SRC_DIR}/winapi.cpp,${SRCS})
SRCS:=$(filter-out ${SRC_DIR}/sendkeys.cpp,${SRCS})
SRCS:=$(filter-out ${SRC_DIR}/reg.cpp,${SRCS})
SRCS:=$(filter-out ${SRC_DIR}/Utf8ConvTest.cpp,${SRCS})
endif

ifeq ($(BUILD_SYS),win_msvc)
MSVC_SLN=webapp.sln
MSVC_SLN_CONS=webapp-console.sln
ifeq ($(MAKECMDGOALS),msvc_cons)
MSVC_SLN=${MSVC_SLN_CONS}
endif
else
ifeq ($(MAKECMDGOALS),gcc_cons)
TARGET_DIR=${TARGET_DIR_CONS}
CPPFLAGS:=-D_CONSOLE $(CPPFLAGS)
LDFLAGS:=$(filter-out -mwindows,$(LDFLAGS))
endif
endif

OBJS=$(SRCS:.cpp=.o) $(RES_SRC:.rc=.o)
OBJS:=$(subst ${SRC_DIR}/,${TARGET_DIR}/,${OBJS})
TARGET=${TARGET_DIR}/${PREFIX}${EXEXT}
TARGET_CONS=${TARGET_DIR_CONS}/c${PREFIX}${EXEXT}
ifeq ($(MAKECMDGOALS),gcc_cons)
TARGET=${TARGET_CONS}
endif
.PHONY: FORCE


# Definition des variables
ifeq ($(BUILD_SYS),win_msvc)
TARGET_CONS=${TARGET_DIR_CONS}/c${PREFIX}${EXEXT}
ifeq ($(MAKECMDGOALS),msvc_cons)
TARGET=${TARGET_CONS}
endif
TARGETS=version_check.txt src/version.h src/werr_map.h src/wm_map.h ${SRC_DIR}/${PREFIX}.ico ${TARGET} # README.docx webapp_quick_user_doc_and_reference.pdf
else
OBJS:=$(filter-out ${TARGET_DIR}/getopt.o,${OBJS})
TARGETS=version_check.txt src/version.h ${TARGET}
endif

# Cibles génériques
all : ${TARGETS}

webapp_quick_user_doc_and_reference.pdf : webapp_quick_user_doc_and_reference.docx
	pandoc -o $@ -V geometry:landscape -f docx -t pdf $<
 
webapp_tutorial.pdf : webapp_tutorial.md
	@echo "Converting $< to $@"
	@pandoc -o $<.html --pdf-engine=xelatex $<
	@pandoc -o $@ -V geometry:margin=0.4cm --pdf-engine=xelatex $<.html 2>/dev/null | true
#	@rm $<.html
#	@pandoc -o $@ -V geometry:landscape --pdf-engine=xelatex $<.html 2>/dev/null | true

ifeq ($(BUILD_SYS),win_msvc)
msvc : ${TARGETS}

msvc_cons : ${TARGETS}
endif

gcc : ${TARGETS}

gcc_cons : ${TARGET}

w64 : ${TARGETS}


# Définition des cibles spécifiques
ifeq ($(BUILD_SYS),win_msvc)
#.PRECIOUS: werr_map.h wm_map
src/werr_map.h : scripts/gen_werr_map.sh
	@echo "Generating $@"
	@./$< >$@

src/wm_map.h : scripts/gen_wm_map.sh
	@echo "Generating $@"
	@./$< >$@

${TARGET} : ${SRC_DIR}/version.h ${SRC_DIR}/${PREFIX}.ico ${SRCS} ${RES_SRC}
	@${MSBUILD} ${MSVC_SLN} -p:Configuration=${CONF}
	@echo "${TARGET} OK"
else
${TARGET} : ${OBJS}
	@mkdir -p ${TARGET_DIR}
	$(LINK.cc) ${OBJS} $(LOADLIBES) $(LDLIBS) -o $@
endif

${TARGET_DIR}/${PREFIX}_res.o : ${SRC_DIR}/${PREFIX}.ico

ifeq (${TARGET_API},windows)
ifeq ($(BUILD_SYS),win_msvc)
strip : msvc msvc_cons
	@${MAKE} -s msvc_cons
else
strip : gcc gcc_cons
	@${MAKE} -s gcc_cons
endif
	@( $(STRIP) ${TARGET_CONS} | true  ) >/dev/null 2>&1
else
strip : ${TARGET}
endif
	@( $(STRIP) ${TARGET} | true  ) >/dev/null 2>&1
ifeq (${TARGET_API},windows)
	@( $(STRIP) build/gcc/win/mingw64/webapp.exe | true  ) >/dev/null 2>&1
	@( $(STRIP) build/gcc/win_cons/mingw64/cwebapp.exe | true  ) >/dev/null 2>&1
endif

upx : strip
	@( ${UPX} ${TARGET} | true  ) >/dev/null 2>&1
ifeq (${TARGET_API},windows)
	@( $(UPX) ${TARGET_CONS} | true  ) >/dev/null 2>&1
endif

${ASSETS}/${PREFIX}-${VERSION}-${SYS_VER}.zip : upx
	@rm -f $@
ifeq (${TARGET_API},windows)
	@zip -qj $@ ${TARGET} build/msvc/win_cons/x64/Release/c${PREFIX}${EXEXT}
else
	@zip -qj $@ ${TARGET}
endif
	@echo "Package $@ is ready"

setup : ${ASSETS}/${PREFIX}-${VERSION}-${SYS_VER}.zip

deliv : ${ASSETS}/${PREFIX}-${VERSION}-${SYS_VER}.zip
	@echo "Delivering it to github."
	@./scripts/github_release.sh $<
#	@echo "Delivering it to gitlab"
#	@./scripts/gitlab_release.sh $< ${PREFIX} ${VERSION}


clean :
	rm -f *~ ${SRC_DIR}/${PREFIX}.ico ${ASSETS}/README.docx ${ASSETS}/${PREFIX}-${VERSION}-${SYS_VER}.zip # ${PREFIX}*.vcxproj.user
ifeq (${TARGET_API},windows)
	rm -rf build/gcc/win build/gcc/win_cons build/msvc .vs
else
	rm -rf ${TARGET_DIR}
endif

include rules.mk

