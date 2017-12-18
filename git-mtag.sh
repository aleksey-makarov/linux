# set -x

mtag_open() {

	if [ -z "$1" ] ; then
		echo "tag name???"
		exit 1
	fi

	tag_name="$1"

	if [ -e mtags/${tag_name} ] ; then
		echo "tag exists"
		exit 1
	fi

	mkdir -p mtags
	echo begin > mtags/${tag_name}
	git add mtags/${tag_name}
	git commit -m "<<<<<< $*"
}

mtag_close() {

	if [ -z "$1" ] ; then
		echo "tag name???"
		exit 1
	fi

	tag_name="$1"

	if [ ! -f mtags/${tag_name} ] ; then
		echo "open tag does not exist"
		exit 1
	fi

	if [ `cat mtags/${tag_name}` != "begin" ] ; then
		echo "wrong open tag \"`cat mtags/${tag_name}`\" != ..."
		exit 1
	fi

	echo end >> mtags/${tag_name}
	git add mtags/${tag_name}
	git commit -m ">>>>>> $*"
}


if [ $# == 0 ] ; then
	echo 'command???'
	exit 1
else
	cmd=$1
	shift

	case $cmd in

		"open" )
			mtag_open  $*
			;;
		"close" )
			mtag_close $*
			;;
		"pair" )
			mtag_open  $*
			mtag_close $*
			;;
		*)
			echo "unknown command $1"
			exit 1
			;;

	esac
fi
