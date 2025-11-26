#!/usr/bin/env bash
#
# find_and_sync_latest_AU.sh - Find and sync the latest AU on Grease
#                              using sync_all.sh.
#
# usage: $0
#
# NOTE: See README for setup and use notes.

set -o errexit

workspace=$(pwd) # TODO: Take as optional input.
script_dir=$(dirname $(readlink -f $0))

source "$script_dir/find_and_sync_latest_AU_lib.sh"

grease_branch=$(cat $grease_branch_file)
grease_user=$(cat $grease_credentials_file | head -1)
grease_password=$(cat $grease_credentials_file | tail -1)
grease_mf_url=$(cat $grease_mf_url_file)
last_au=$(cat $last_au_file)
last_au_name=$(echo $last_au | grep -o '^[^\.]*')

echo "Branch:                 $grease_branch"
echo "Last synced AU:         $last_au"

if [ -d "$workspace/grease-manifest" ]; then
    pushd "$workspace/grease-manifest" &> /dev/null
    git fetch $grease_mf_url refs/heads/$grease_branch &> /dev/null
    latest_au_rev=$(git show FETCH_HEAD | grep -o '[\.0123456789]*\.xml' |
                    head -1 | sed -e 's|\.xml$||')
    popd &> /dev/null
    latest_au="$last_au_name.$latest_au_rev"
    echo "Latest AU:              $latest_au"
fi

if [ "$last_au" != "$latest_au" ]; then
    echo "Syncing $latest_au, see sync_all_log for details."
    $workspace/grease-manifest/bin/sync_all.sh -u $grease_user
        -p $grease_password -a $latest_au -b $grease_branch &>> sync_all_log
fi

echo "$latest_au" > "$last_au_file"

exit 0
