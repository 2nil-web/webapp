#!/bin/bash

SCRIPT=$(realpath "$0")
# <=> SCRIPT="$(readlink --canonicalize-existing "$0")"
SCRIPTPATH=$(dirname "$SCRIPT")
. ${SCRIPTPATH}/gitlab_access.sh

asset_path=$1
PREFIX=$2
VERSION=$3

asset_name=$(basename $1)
repo_git=$(git config --get remote.gitlab.url)
repo_name=$(basename -s .git ${repo_git})
proj_id=$(curl -s --header "PRIVATE-TOKEN: ${gl_tok}" "${gl_api_endp}/projects?owned=true" | jq -r '.[] | .web_url,.id' | sed 'N;s/\n/;/' | sed -n "s/.*${repo_name};//p")

if false; then
echo "repo_git $repo_git"
echo "repo_name $repo_name"
echo "gl_tok $gl_tok"
echo "gl_api_endp $gl_api_endp"
echo "proj_id $proj_id"
echo "gl_user $gl_user"
echo "asset_path $asset_path"
echo "asset_name $asset_name"
fi

curl -s --header "PRIVATE-TOKEN: ${gl_tok}" --upload-file $asset_path "${gl_api_endp}/projects/${proj_id}/packages/generic/${PREFIX}/${VERSION}/$asset_name" >/dev/null
echo "Delivery to gitlab done, to see it go there https://gitlab.com/${gl_user}/${repo_name}/-/packages"

