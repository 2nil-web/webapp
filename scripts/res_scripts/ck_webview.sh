#!/bin/bash

# Create a patch file to go from old file to new file and check it
function create_and_check_patch () {
  old_file=$1
  new_file=$2
  diff "$old_file" "$new_file" >patch_from_old_to_new
  cp "$old_file" test_file
  # Apply patch : patch old_file <patch_from_old_to_new
  patch test_file <patch_from_old_to_new
  # Check that old_file and new_file are now identical ==> command should output nothing and return 0 (echo $?) : diff old_file new_file
  diff test_file "$new_file"
}

#create_and_check_patch wv_ghrepo/webview/webview.h my_webview.h

NEW_COMMIT_FILE=last_webview_commits.txt
NEW_COMMIT_FILE_DIR=$PWD

# Get actual commit of my_webview.h
wv_actual_commit=$(tail -1 last_webview_commits.txt | sed 's/.*commit is //')

# Prepare to get last commit of webview in github
WV_REPO=wv_ghrepo
test -d "${WV_REPO}" || mkdir "${WV_REPO}"
cd "${WV_REPO}"


if [ -d webview ]
then
  cd webview
  git pull
else
  git clone https://github.com/webview/webview.git
  cd webview
fi

# Get last (short) commit of webview.h
wv_new_commit=$(git rev-list --abbrev-commit HEAD core/include/webview/webview.h | head -1)
# git rev-list HEAD --oneline webview.h # with one line comment

#echo "wv_new_commit:${wv_new_commit}"
#echo "wv_actual_commit:${wv_actual_commit}"

if [ "${wv_new_commit}" = "${wv_actual_commit}" ]
then
  echo "No change in webview.h from the github webview repo"
else
  echo "On $(LANG=fr date) webview.h commit is $wv_new_commit" >> ${NEW_COMMIT_FILE_DIR}/${NEW_COMMIT_FILE}
cat << EOF
webview github repo has a new commit, have a look at file '${NEW_COMMIT_FILE}'.
Merge changes in wv_ghrepo/webview/ directory with what is in webview directory .
But as a reminder for commit 2699355 (2024-09-21), on file [wv_ghrepo/]webview/core/include/webview/detail/backends/win32_edge.hh, my changes are:
  line 512-523:
#ifdef MY_WEBVIEW_EXT
#ifndef __GNUC__
      extern
#endif
      void WindowInitialState(HWND hw);
      WindowInitialState(m_window);
#else
      ShowWindow(m_window, SW_SHOW);
      UpdateWindow(m_window);
      SetFocus(m_window);
#endif

  line 725: replace "CSIDL_APPDATA" by "CSIDL_LOCAL_APPDATA"
  line 794-802 may have:
#ifdef MY_WEBVIEW_EXT
#ifndef __GNUC__
      extern
#endif
    bool my_configure(ICoreWebView2 * mwv, ICoreWebView2Controller * mctl);
    my_configure(m_webview, m_controller);
#endif

EOF
#Check /mingw64/include/eventtoken.h and wv_ghrepo/webview/compatibility/mingw/includeEventToken.h, although not useful with msys2/mingw64
fi
