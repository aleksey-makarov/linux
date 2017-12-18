# set -x

. ./setenv.sh

archive_dir=/home/amakarov/work/old_branches

function archive_branch() {
  i=$1
  bundle_filename=${archive_dir}/${i}.bundle
  mkdir -p `dirname ${bundle_filename}`
  git bundle create ${bundle_filename} origin/master..${i} || exit
  pbzip2 ${bundle_filename} || exit
  git branch -D ${i}
}

branches='checksum_fix.*'

for i in `git branch --list $branches` ; do
  echo_log "archiving \"$i\""
  archive_branch $i
  # echo "deleting \"$i\""
  # git branch -D $i
done

exit

# interactively
for i in `git branch` ; do
  echo -n "delete branch \"${i}\" (y/N)? "
  read answer
  if echo "$answer" | grep -iq "^y" ; then
      echo "well, i am going to delete it"
      archive_branch $i
  else
      echo "ok, leave it"
  fi
done

