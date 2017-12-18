# set -x

source ./setenv.sh

$(check_remote)

branch=$(get_remote_branch)

git fetch ${REMOTE}
git checkout -t -b ${branch} ${REMOTE}/${branch}
