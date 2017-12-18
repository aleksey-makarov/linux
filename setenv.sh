export RED=`tput setaf 1`
export GREEN=`tput setaf 2`
export RESET=`tput sgr0`

function echo_log()
{
  echo -n $GREEN
  echo -n "$*"
  echo $RESET
}

function echo_error()
{
  echo -n $RED
  echo -n "$*"
  echo $RESET
}

case `hostname` in
  plum)
    echo_log "plum"
    export P=4
    export REMOTE_SSH_ADDR=amakarov@amakarov.ddns.net
    export REMOTE_SSH_DIR=/home/amakarov/work/linux
    export REMOTE=lemon
  ;;
  pomelo)
    echo_log "pomelo"
    export P=4
    export REMOTE_SSH_ADDR=amakarov@lemon
    export REMOTE_SSH_DIR=/home/amakarov/work/linux
    export REMOTE=lemon
  ;;
  lemon)
    echo_log "lemon"
    export P=6
  ;;
  *)
    echo_error "Where am I?"
  ;;
esac

export ARCH=arm64
# export PATH=/home/amakarov/work/gcc-linaro/bin:${PATH}
export CROSS_COMPILE=aarch64-linux-gnu-
export INSTALL_MOD_PATH=/home/amakarov/work/linux.install
export PYTHONCMD=python2
export LANG=C

function check_remote ()
{
  if [ -z ${REMOTE+x} ]; then
    echo "REMOTE is unset"
    exit
  fi

  if [ -z ${REMOTE_SSH_ADDR+x} ]; then
    echo "REMOTE_SSH_ADDR is unset"
    exit
  fi

  if [ -z ${REMOTE_SSH_DIR+x} ]; then
    echo "REMOTE_SSH_DIR is unset"
    exit
  fi
}

function get_timestamp ()
{
  date '+%y%m%d%H%M%S'
}

function get_remote_branch ()
{
  $(check_remote)
  ssh ${REMOTE_SSH_ADDR} "cd ${REMOTE_SSH_DIR} && git rev-parse --abbrev-ref HEAD"
}

function mk()
{
  echo_log `which ${CROSS_COMPILE}gcc`
  make -j${P}
}
