# set -x

source ./setenv.sh

$(check_remote)

branch=$(get_remote_branch)
timestamp=$(get_timestamp)
tmp_branch_name=${branch}.${timestamp}

git branch ${tmp_branch_name}
git push ${REMOTE} ${tmp_branch_name}:${tmp_branch_name}
ssh ${REMOTE_SSH_ADDR} "cd ${REMOTE_SSH_DIR} && git merge ${tmp_branch_name}"
ssh ${REMOTE_SSH_ADDR} "cd ${REMOTE_SSH_DIR} && git branch -d ${tmp_branch_name}"
git branch -d ${tmp_branch_name}
