remote=alarm
remote_ssh=alarm
remote_dir=/home/amakarov/linux

current_branch=`git rev-parse --abbrev-ref HEAD`

echo "Current branch: ${current_branch}"

git push $remote ${current_branch}:${current_branch}
ssh alarm "cd ${remote_dir} ; ./build_branch.sh ${current_branch}"
