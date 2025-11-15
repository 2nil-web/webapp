#!/bin/bash

function gen_werr_map() {
# To find the mingw64 include counterpart in msvc, run:
# find 'C:/Program Files (x86)/Windows Kits/10/Include' -iname winerror.h
MSC_WE_INC='C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/shared/winerror.h'
GCC_WE_INC='/mingw64/include/winerror.h'

DEF_HEAD=WIN_ERROR_MAP_H

if [ -f "${MSC_WE_INC}" ]
then
  WE_INC="${MSC_WE_INC}"
else
  WE_INC="${GCC_WE_INC}"
fi

sed -n 's/^#define \(ERROR_\([^ ]*\)\) .*(\(.*\))$/\1 \L\2 \3/p' "${WE_INC}" | sort -u | while read idx str val
do
  str=${str//_/ }
  (
  echo "#ifndef ${idx}"
  echo "#define ${idx} ${val// / /}"
  echo "#endif"
  echo
  ) >>werr_def.tmp.h
  echo "  {${idx}, \"${str}\"}," >>werr_map.tmp.h
done


cat <<EOF1
#ifndef ${DEF_HEAD}
#define ${DEF_HEAD}

/* Automatically generated from the file ${WE_INC} */

#include <map>
#include <string>
#include <windows.h>
#include <winerror.h>

EOF1
cat werr_def.tmp.h

echo "std::map<DWORD, std::string> werr_map = {"
cat werr_map.tmp.h
echo -e "};\n\n#endif /* ${DEF_HEAD} */\n"

rm -f werr_def.tmp.h werr_map.tmp.h
}

gen_werr_map

