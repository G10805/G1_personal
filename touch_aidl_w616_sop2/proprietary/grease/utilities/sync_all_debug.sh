#!/bin/bash

usage() {
  echo "
      Usage: sh `readlink -f $0` \\
            -u grease-username \\
            -p grease-password \\
            -a au \\
            -b grease-branch \\
            -c caf-manifest \\
            -l caf-url" \\
  exit 1
}

grease_server="grease.qualcomm.com"
while getopts u:p:a:b:c:g:l:m: o
do
    case "$o" in
        u) grease_user="$OPTARG";;
        p) grease_pass="$OPTARG";;
        a) full_au="$OPTARG";;
        b) branch="$OPTARG";;
        c) caf_manifest_repo="$OPTARG";;
        l) caf_server_url="$OPTARG";;
        [?]) usage ;;
    esac
done


if [ -z "$grease_user" ]; then

  echo "Please supply your grease username."
  usage

fi

if [ -z "$grease_pass" ]; then

  echo "Please supply your grease password."
  usage

fi

if [ -z "$full_au" ]; then

  echo "Please supply the full AU name."
  usage

fi

if [ -z "$branch" ]; then

  echo "Please supply the branch name."
  usage

fi

if [ -z "$caf_manifest_repo" ]; then
  caf_manifest_repo="platform/manifest"
fi

if [ -z "$caf_server_url" ]; then
  caf_server_url="git://codeaurora.org"
fi

grease_util_dir=$(readlink -f "$(dirname "$0")")

# Colect debug logs in debug dir
if [[ -d "$grease_util_dir/debug" ]]; then

    rm -rf "$grease_util_dir/debug"

fi

mkdir "$grease_util_dir/debug"

echo "Gathering workspace and grease util dir information..."

    pwd 2>&1 | tee "$grease_util_dir/debug/pwd_log.txt"

    ls -laR 2>&1 |
        tee "$grease_util_dir/debug/pwd_ls_log.txt"

    ls -laR "$grease_util_dir/" 2>&1 |
        tee "$grease_util_dir/debug/util_dir_ls_log.txt"


echo "checking xmlstarlet installation..."

    (

         if command -v xmlstarlet &>/dev/null; then
             echo "Present"
         else
             echo "Absent"
         fi

    ) | tee "$grease_util_dir/debug/xmlstarlet_log.txt"


echo "Executing sync_all.sh in trace mode"

    sync_all_cmd="sync_all.sh"
    caf_manifest_dir="caf-manifest"
    grease_manifest_dir="grease-manifest"

    if [ ! -f $sync_all_cmd ]; then

        sync_all_cmd="${grease_util_dir}/$sync_all_cmd"
        caf_manifest_dir="${grease_util_dir}/$caf_manifest_dir"
        grease_manifest_dir="${grease_util_dir}/$grease_manifest_dir"

    fi

    bash -x $sync_all_cmd -a $full_au \
                          -b $branch \
                          -u $grease_user \
                          -p $grease_pass \
                          -c $caf_manifest_repo \
                          -l $caf_server_url 2>&1 |
            tee "$grease_util_dir/debug/sync_all_log.txt"


echo "Collecting generated caf and grease manifests"

    caf_mf_name="caf_${full_au}.xml"

    au="$(echo $full_au |
        sed 's/^.*\([0-9]\{2\}\.[0-9]\{2\}\.[0-9]\{2\}\.[0-9]\{2,\}\.[0-9]\{3\}\).*/\1/')"
    grease_mf_name="${au}.xml"

    cp $caf_manifest_dir/$caf_mf_name \
       $grease_manifest_dir/$grease_mf_name "$grease_util_dir/debug/"


echo "Collecting configuration data..."

    cat ~/.ssh/id_rsa.pub 2>&1 | tee "$grease_util_dir/debug/id_rsa.txt"
    cat ~/.ssh/config 2>&1 | tee "$grease_util_dir/debug/ssh_config.txt"
    cat ~/.gitconfig 2>&1 | tee "$grease_util_dir/debug/gitconfig.txt"
    cat /etc/gitconfig 2>&1 | tee "$grease_util_dir/debug/etc_gitconfig.txt"


echo "Collecting caf ssh info..."

    caf_host=$(sed -rne 's:^.*\://([^/]*)/?.*$:\1:p' <<<$caf_server_url)
    caf_server=$(echo $caf_host | cut -d':' -f1)
    if [[ $caf_host =~ .+\:.+ ]]; then
        caf_port=$(echo $caf_host | cut -d':' -f2)
        ssh -vvv -p $caf_port $caf_server info 2>&1 |
            tee "$grease_util_dir/debug/${caf_server}_${caf_port}.txt"
    fi

    ssh -vvv -p 22 $caf_server info 2>&1 |
        tee "$grease_util_dir/debug/${caf_server}_22.txt"

    default_caf_remote=$(xmlstarlet sel \
        -t -v /manifest/default/@remote ${caf_manifest_dir}/${caf_mf_name})

    caf_url=$(xmlstarlet sel \
        -t -v "/manifest/remote[@name='$default_caf_remote']/@fetch" \
        $caf_manifest_dir/$caf_mf_name)

    prjs_caf_host=$(sed -rne 's:^.*\://([^/]*)/?.*$:\1:p' <<<${caf_server_url})
    prjs_caf_server=$(echo $prjs_caf_host | cut -d':' -f1)

    if [ $prjs_caf_server != $caf_server ]; then

        if [[ $prjs_caf_host =~ .+\:.+ ]]; then
            prjs_caf_port=$(echo $prjs_caf_host | cut -d':' -f2)
            ssh -vvv -p $prjs_caf_port $prjs_caf_server info 2>&1 |
                tee "$grease_util_dir/debug/${prjs_caf_server}_${prjs_caf_port}.txt"
        fi

        ssh -vvv -p 22 $prjs_caf_server info 2>&1 |
            tee "${grease_util_dir}/debug/${prjs_caf_server}_22.txt"

    fi
