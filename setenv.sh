export ARCH=arm64
export PATH=/home/amakarov/work/gcc-linaro/bin:${PATH}
export CROSS_COMPILE=aarch64-linux-gnu-
export INSTALL_MOD_PATH=/home/amakarov/work/linux.install
export P=6
export PYTHONCMD=python2
export LANG=C

export RED=`tput setaf 1`
export GREEN=`tput setaf 2`
export RESET=`tput sgr0`

function get_timestamp ()
{
  date '+%y%m%d%H%M%S'
}

function echo_log()
{
	echo -n $GREEN
	echo -n "$*"
	echo $RESET
}
