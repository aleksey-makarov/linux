# set -x

source ./setenv.sh

$(check_remote)

timestamp=$(get_timestamp)
branch=`git rev-parse --abbrev-ref HEAD`
tmp_branch_name=${branch}.${timestamp}

git branch ${tmp_branch_name}
git push ${REMOTE} ${tmp_branch_name}:${tmp_branch_name}
ssh ${REMOTE_SSH_ADDR} "cd ${REMOTE_SSH_DIR} && git checkout ${tmp_branch_name}"
git branch -D ${tmp_branch_name}
