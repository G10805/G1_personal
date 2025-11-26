#!/usr/bin/env bash
#
# setup_config.sh - Setup config files for find_and_sync_latest_AU.sh
#
# usage: $0 -u [username] -p [password] -a [au-name] -b [branch]

workspace=$(pwd) # TODO: Take as optional input.
script_dir=$(dirname $(readlink -f $0))
source "$script_dir/find_and_sync_latest_AU_lib.sh"

while getopts u:p:a:b: o
do
  case "$o" in
  u) user="$OPTARG";;
  p) pass="$OPTARG";;
  a) full_au="$OPTARG";;
  b) branch="$OPTARG";;
  esac
done

if [ -z "$user" ] || [ -z "$pass" ] || [ -z "$full_au" ] ||
   [ -z "$branch" ]; then
    echo "Please input required arguments."
    exit 1
fi

grease_manifest_repo=platform/manifest
echo $full_au | grep -q GECKO
if [ "$?" == "0" ]; then
    grease_manifest_repo=b2g/manifest
fi

# Populate config files.
mkdir -p "$config_dir"
echo "$branch" > "$grease_branch_file"
echo "$user" > "$grease_credentials_file"
chmod go-rw "$grease_credentials_file"
echo "$pass" >> "$grease_credentials_file"
grease_mf_url="ssh://$user@grease.qualcomm.com:29418/$grease_manifest_repo"
echo "$grease_mf_url" > "$grease_mf_url_file"
echo "$full_au" | sed -e 's|[0123456789]$|0|' > "$last_au_file"

# Setup grease-manifest dir as expected by find_and_sync_latest_AU.sh.
pushd "$workspace" &> /dev/null
mkdir -p grease-manifest
pushd grease-manifest &> /dev/null
git init &> /dev/null
git fetch $grease_mf_url refs/heads/$branch &> /dev/null
git checkout -f FETCH_HEAD &> /dev/null
popd &> /dev/null
popd &> /dev/null

exit 0
