#!/bin/bash
set -o errexit -o pipefail

[ $# -lt 2 ] && { echo "ERROR: specify two manifest files to merge" ; exit ; }
m1=$1 ; m2=$2
shift 2
[ $# -gt 0 ] && { echo "ERROR: don't know what to do with $@" ; exit ; }

# Gets a value for a default name and inserts where that name is not
# currently defined.
remove_default() { # name < xml > xml
  local val name=$1 xml=$(cat)

  val=$(echo "$xml" | xmlstarlet sel -t -v "/manifest/default/@$name")
  if [ -n "$val" ]; then
    echo "$xml" | xmlstarlet ed -i "/manifest/*[not(self::remote|@$name)]"\
                                   -t attr -n "$name" -v "$val"
  fi
}

# Gets each default and inserts it where none is currently defined.
# Deletes all the defaults.
remove_defaults() { # < xml > xml
  local xml=$(cat)

  xml=$(echo "$xml" | remove_default "remote")
  xml=$(echo "$xml" | remove_default "revision")
  # xml=$(echo "$xml" | remove_default "sync-j") # doesn't like the '-'

  echo "$xml" | xmlstarlet ed -d "/manifest/default"
}

# Uses the original remote name to lookup and then replaces it with the
# conflict free remote name.
#   You can't insert if it already exists, update instead.
#   Ensure that we update all node types, not just projects.
update_remote() { # orig new < xml > xml
  local orig new

  orig=$1
  new=$2
  xmlstarlet ed -u "/manifest/remote[@name='$orig']/@name" -v "$new"\
                -u "/manifest/*[@remote='$orig']/@remote" -v "$new"
}

# Since the defaults will likely conflict (like remote), delete them.
# Must remove the defaults before updating so that the default remote
# is updated correctly.
xml1=$(cat "$m1" | remove_defaults)
xml2=$(cat "$m2" | remove_defaults)

m1remotes=($(cat "$m1" |\
             xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
             ))
m2remotes=($(cat "$m2" |\
             xmlstarlet sel -t -m '/manifest/remote' -v '@name' -o ' '\
             ))
for m1remote in "${m1remotes[@]}"; do
  for m2remote in "${m2remotes[@]}"; do
    if [ "$m1remote" = "$m2remote" ]; then
      # Conflict found, update the remote definitions
      # TODO: If the remote definitions are identical, no need to rename
      xml1=$(echo "$xml1" | update_remote "$m1remote" "${m1remote}1")
      xml2=$(echo "$xml2" | update_remote "$m2remote" "${m2remote}2")
    fi
  done
done

{
  echo '<?xml version="1.0" encoding="UTF-8"?>
        <manifest>'

  echo "$xml1" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'
  echo "$xml2" | xmlstarlet sel -t -c '/manifest/*[not(self::project)]'

  echo "$xml1" | xmlstarlet sel -t -c "/manifest/project"
  echo "$xml2" | xmlstarlet sel -t -c "/manifest/project"

  echo '</manifest>'
} |  xmlstarlet fo
