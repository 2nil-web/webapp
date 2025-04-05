#!/bin/bash


function gen_wm_map() {
# To find the mingw64 include counterpart in msvc, run:
# find 'C:/Program Files (x86)/Windows Kits/10/Include' -iname winuser.h
MSC_WM_INC='C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um/WinUser.h'
GCC_WM_INC='/mingw64/include/winerror.h'

DEF_HEAD=WIN_MESSAGE_MAP_H

if [ -f "${MSC_WM_INC}" ]
then
  WM_INC="${MSC_WM_INC}"
else
  WM_INC="${GCC_WM_INC}"
fi

sed -n 's/^#define \(WM_\([^ ]*\)\) *\(.*\)$/\1 \L\2 \3/p' "${WM_INC}" | sort -u | while read idx str val
do
  str=${str//_/ }
  (
  echo "#ifndef ${idx}"
  echo "#define ${idx} ${val// / /}"
  echo "#endif"
  echo
  ) >>wm_def.tmp.h
  echo "  {${idx}, \"${str}\"}," >>wm_map.tmp.h
done


cat <<EOF1
#ifndef ${DEF_HEAD}
#define ${DEF_HEAD}

/* Automatically generated from the file ${WM_INC} */

#include <map>
#include <string>
#include <windows.h>
#include <winuser.h>

EOF1
cat wm_def.tmp.h

echo "std::map<DWORD, std::string> wm_map = {"
cat wm_map.tmp.h
echo -e "};\n\n#endif /* ${DEF_HEAD} */\n"

rm -f wm_def.tmp.h wm_map.tmp.h
}

gen_wm_map
