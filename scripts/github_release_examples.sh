#!/bin/bash


# See managing releases: https://docs.github.com/en/rest/releases/releases
# See managing assets: https://docs.github.com/fr/rest/releases/assets

. ./github_release_access.sh

gh_rel_endp="${gh_api_endp}/repos/${gh_user}/${gh_repo}/releases"

# List releases
curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp}
# List release names and ids
curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp} | jq -r '.[]|.name,.id' | sed 'N;s/\n/;/'
# May add the following properties to the jq command: url,assets_url,upload_url,html_url

# Create a release
tag_name=0.1.4
rel_name=webapp-${tag_name}
if false; then
rel_post="{\"tag_name\":\"${tag_name}\",\"target_commitish\":\"master\",\"name\":\"${rel_name}\",\"body\":\"Description of the release\",\"draft\":false,\"prerelease\":false,\"generate_release_notes\":false}"
curl -s -X POST -H "Authorization: Bearer ${gh_tok}" ${gh_rel_endp} -d "${rel_post}" #>github.release.${rel_name}.json
fi

# Get a specific release id
RELEASE_ID=$(curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp} | jq -r '.[]|.name,.id' | sed 'N;s/\n/;/' | sed -n "s/${rel_name};//p")
# Or release url
RELEASE_URL=$(curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp} | jq -r '.[]|.name,.url' | sed 'N;s/\n/;/' | sed -n "s/${rel_name};//p")
if false; then
# Delete a release
curl -X DELETE -H "Authorization: Bearer ${gh_tok}" ${gh_rel_endp}/${RELEASE_ID}
# OR
curl -X DELETE -H "Authorization: Bearer ${gh_tok}" ${RELEASE_URL}
fi

# List release assets
gh_upl_endp=$(curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp}/${RELEASE_ID}/assets)

# Get a specific release upload_url
gh_upl_url=$(curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp} | jq -r '.[]|.name,.upload_url' | sed 'N;s/\n/;/;s/{?name,label}//' | sed -n "s/${rel_name};//p")

if false; then
# Upload a release asset
asset_name=webapp-0.1.4-Windows_NT_10.0.19045.zip
curl -s -X POST -H "Authorization: Bearer ${gh_tok}" -H "Content-Type: application/octet-stream" ${gh_upl_url}?name=${asset_name} --data-binary "@${asset_name}"
fi

# Get a specific a specific asset id
ASSET_ID=$(curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp}/${RELEASE_ID}/assets | jq -r '.[]|.name,.id' | sed 'N;s/\n/;/' | sed -n "s/${asset_name};//p")

# Get a release asset
curl -H "Authorization: Bearer ${gh_tok}" ${gh_rel_endp}/assets/${ASSET_ID}

if false; then
# Update a release asset
asset_name=webapp-Windows_NT_10.0.19045.zip
asset_post="{\"name\":\"webapp-Windows_NT_10.0.19045.zip\",\"label\":\"Windows binary\"}"
curl -X PATCH -H "Authorization: Bearer ${gh_tok}" ${gh_rel_endp}/assets/${ASSET_ID} -d ${asset_post}
fi

