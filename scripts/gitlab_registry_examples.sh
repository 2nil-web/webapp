#!/bin/bash

# From : https://docs.gitlab.com/ee/api/packages.html

. ./gitlab_registry_access.sh

# Get Project ID by clicking on the 3 vertical dots at the upper right side of the repo front page
# ...

# REST API command to detail all my own projects
curl --header "PRIVATE-TOKEN:  ${gl_tok}" "${gl_api_endp}/projects?owned=true" | jq

# REST API command to list the web url and project id of all my own projects
curl -s --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects?owned=true" | jq -r '.[] | .web_url,.id' | sed 'N;s/\n/;/'

# REST API command to get the project id of the current repository
repo_name=$(basename -s .git `git config --get remote.origin.url`)
proj_id=$(curl -s --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects?owned=true" | jq -r '.[] | .web_url,.id' | sed 'N;s/\n/;/' | sed -n "s/.*${repo_name};//p")

VERSION=0.1.4
PACK_FILE=webapp-0.1.4-Windows_NT_10.0.19045.zip
# REST API command to upload a package to the registry of repository
if false; then
curl --header "PRIVATE-TOKEN: ${gl_tok}" --upload-file ${PACK_FILE} ${gl_api_endp}/projects/${proj_id}/packages/generic/${repo_name}/${VERSION}/${PACK_FILE}
fi

# REST API command to detail all available packages registry for a given repository
curl -s --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects/${proj_id}/packages" | jq

# REST API command to get the package id for a given repository
pack_id=$(curl -s --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects/${proj_id}/packages" | jq -r '.[] | .name,.id' | sed 'N;s/\n/;/' | sed -n "s/${repo_name};//p")

# REST API command to get all the package files details of a single package.
curl -s --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects/${proj_id}/packages/${pack_id}/package_files" | jq

# REST API command to get the package files creation time and id of a given package.
curl -s --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects/${proj_id}/packages/${pack_id}/package_files" | jq -r '.[] | .created_at,.id' | sed 'N;s/\n/;/'

# Delete registry repo
if false; then
curl --request DELETE --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects/${proj_id}/packages/${pack_id}"
fi

# Get the oldest package file id of a given package
curl -s --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects/${proj_id}/packages/${pack_id}/package_files" | jq -r '.[] | .created_at,.id' | sed 'N;s/\n/;/' | sort | sed 's/.*;//' | head -1
# sort -r for the newest

# Delete a package file of a given registry repo
if false; then
curl --request DELETE --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects/${proj_id}/packages/${pack_id}/package_files/<package_file_id>"
fi

