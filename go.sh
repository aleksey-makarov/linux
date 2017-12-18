# set -x

DATE=`date '+%y%m%d%H%M%S'`

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

RED=`tput setaf 1`
GREEN=`tput setaf 2`
RESET=`tput sgr0`

# ----------------------------------------------------------------------------
# submit
# ----------------------------------------------------------------------------

# FORMAT_PATCH_TAG=acpi-amba
# FORMAT_PATCH_ID=31
# FORMAT_VERSION=6

# FORMAT_PATCH_TAG=acpi-console
# FORMAT_PATCH_ID=a7
# FORMAT_VERSION=11 # [PATCH vXXX]

# FORMAT_PATCH_TAG=acpi-upgrade
# FORMAT_PATCH_ID=18
# FORMAT_VERSION=4 # [PATCH vXXX]

FORMAT_PATCH_TAG=ptp
FORMAT_PATCH_ID=19
FORMAT_VERSION=6 # [PATCH vXXX]

# FORMAT_PATCH_TAG=mbox
# FORMAT_PATCH_ID=01
# FORMAT_VERSION=2 # [PATCH vXXX]

submit_format() {

	OG_LN1=outgoing.${FORMAT_PATCH_TAG}
	OG_LN2=${OG_LN1}.${FORMAT_PATCH_ID}
	OG=${OG_LN2}.${DATE}
	COVER_FILE=mtags/${FORMAT_PATCH_TAG}.cover
	REVS=`git blame mtags/${FORMAT_PATCH_TAG} | awk '\
		/begin$/ { b=$1 } \
		/end$/ { e=$1 } \
		END { print (b ".." e "^"); } '`

	if [ ! -e ${COVER_FILE} ] ; then
		echo "no ${COVER_FILE} file"
		return
	fi

	if [[ -z ${FORMAT_VERSION} ]] ; then
		FORMAT_VERSION_ARG=""
	else
		FORMAT_VERSION_ARG="-v ${FORMAT_VERSION}"
	fi

	git format-patch -M \
		`#--subject-prefix='RFC'` \
		--subject-prefix='PATCH net-next' \
		`#--subject-prefix='PATCH net'` \
		`#--from='Aleksey Makarov <aleksey.makarov@auriga.com>'` \
		${FORMAT_VERSION_ARG} \
		--cover-letter \
		-o ${OG} \
		$REVS

	COVER_PATCH=${OG}/*-cover-letter.patch

	COVER_HEAD=$(head -n 1 mtags/${FORMAT_PATCH_TAG}.cover)
	sed -i -e "s/\*\*\* SUBJECT HERE \*\*\*/${COVER_HEAD}/" $COVER_PATCH

	COVER_BODY_TMP_FILE=./_tmp_b_${DATE}
	tail -n +3 $COVER_FILE > $COVER_BODY_TMP_FILE
	sed -i -e "/\*\*\* BLURB HERE \*\*\*/r _tmp_b_${DATE}" -e "/\*\*\* BLURB HERE \*\*\*/d" $COVER_PATCH

	rm $COVER_BODY_TMP_FILE
	ln -T -f -s $OG $OG_LN2
	ln -T -f -s $OG_LN2 $OG_LN1
}

submit_submit() {

	# password: "tmkcricennddzbff"

	# -------------------------------------------------------
	# local
	# -------------------------------------------------------

# git send-email --no-chain-reply-to --suppress-cc=all \
# 	`#--dry-run` \
# 	--to="Linu Cherian <linu.cherian@cavium.com>" \
# 	--cc='"Goutham, Sunil" <Sunil.Goutham@cavium.com>' \
# 	--cc="Aleksey Makarov <aleksey.makarov@cavium.com>" \
# 	outgoing/*

#	--to="Aleksey Makarov <aleksey.makarov@auriga.com>" \


	# -------------------------------------------------------
	# send one patch
	# -------------------------------------------------------

#git send-email --cc-cmd='./scripts/get_maintainer.pl --interactive --no-git-fallback' --no-chain-reply-to \
#	`#--dry-run` \
#	--in-reply-to='20170315102854.1763-1-aleksey.makarov@linaro.org' \
#	--to="linux-serial@vger.kernel.org" \
#	--cc="linux-kernel@vger.kernel.org" \
#	--cc="Aleksey Makarov <aleksey.makarov@linaro.org>" \
#	--cc="Sudeep Holla <sudeep.holla@arm.com>" \
#	--cc="Greg Kroah-Hartman <gregkh@linuxfoundation.org>" \
#	--cc="Peter Hurley <peter@hurleysoftware.com>" \
#	--cc="Jiri Slaby <jslaby@suse.com>" \
#	--cc="Robin Murphy <robin.murphy@arm.com>" \
#	--cc="Steven Rostedt <rostedt@goodmis.org>" \
#	--cc='"Nair, Jayachandran" <Jayachandran.Nair@cavium.com>' \
#	--cc="Sergey Senozhatsky <sergey.senozhatsky@gmail.com>" \
#	--cc="Petr Mladek <pmladek@suse.com>" \
#	./outgoing/v8-0003-printk-fix-double-printing-with-earlycon.patch

# -------------------------------------------------------
# PTP
# -------------------------------------------------------

#git send-email --cc-cmd='./scripts/get_maintainer.pl --interactive --no-git-fallback' --no-chain-reply-to \
#	`#--dry-run` \
#	--to="netdev@vger.kernel.org" \
#	--cc="linux-arm-kernel@lists.infradead.org" \
#	--cc="linux-kernel@vger.kernel.org" \
#	--cc='"Goutham, Sunil" <Sunil.Goutham@cavium.com>' \
#	--cc="Radoslaw Biernacki <rad@semihalf.com>" \
#	--cc="Aleksey Makarov <aleksey.makarov@cavium.com>" \
#	--cc="Robert Richter <rric@kernel.org>" \
#	--cc="David Daney <ddaney@caviumnetworks.com>" \
#	--cc="Richard Cochran <richardcochran@gmail.com>" \
#	--cc="Philippe Ombredanne <pombredanne@nexb.com>" \
#	outgoing/*

	# -------------------------------------------------------
	# console2
	# -------------------------------------------------------

#git send-email --cc-cmd='./scripts/get_maintainer.pl --interactive --no-git-fallback' --no-chain-reply-to \
#	`#--dry-run` \
#	--to="linux-serial@vger.kernel.org" \
#	--cc="linux-kernel@vger.kernel.org" \
#	--cc="Aleksey Makarov <aleksey.makarov@linaro.org>" \
#	--cc="Sudeep Holla <sudeep.holla@arm.com>" \
#	--cc="Greg Kroah-Hartman <gregkh@linuxfoundation.org>" \
#	--cc="Peter Hurley <peter@hurleysoftware.com>" \
#	--cc="Jiri Slaby <jslaby@suse.com>" \
#	--cc="Robin Murphy <robin.murphy@arm.com>" \
#	--cc="Steven Rostedt <rostedt@goodmis.org>" \
#	--cc='"Nair, Jayachandran" <Jayachandran.Nair@cavium.com>' \
#	--cc="Sergey Senozhatsky <sergey.senozhatsky@gmail.com>" \
#	--cc="Petr Mladek <pmladek@suse.com>" \
#	outgoing/*

	# -------------------------------------------------------
	# ACPI_TABLE_UPGRADE
	# -------------------------------------------------------

# git send-email --cc-cmd='./scripts/get_maintainer.pl --interactive --no-git-fallback' --no-chain-reply-to \
# 	`#--dry-run` \
# 	--to="Rafael J. Wysocki <rjw@rjwysocki.net>" \
#   --cc="Catalin Marinas <catalin.marinas@arm.com>" \
#   --cc="Will Deacon <will.deacon@arm.com>" \
# 	--cc="Len Brown <lenb@kernel.org>" \
# 	--cc="linux-acpi@vger.kernel.org" \
# 	--cc="linux-arm-kernel@lists.infradead.org" \
# 	--cc="linux-kernel@vger.kernel.org" \
# 	--cc="Aleksey Makarov <aleksey.makarov@linaro.org>" \
# 	--cc="Graeme Gregory <graeme.gregory@linaro.org>" \
# 	--cc="Jon Masters <jcm@redhat.com>" \
# 	--cc='"Zheng, Lv" <lv.zheng@intel.com>' \
# 	--cc="Mark Rutland <mark.rutland@arm.com>" \
# 	outgoing/*

	# -------------------------------------------------------
	# ACPI/DBG2
	# -------------------------------------------------------

#	git send-email --cc-cmd='./scripts/get_maintainer.pl --interactive --no-git-fallback' --no-chain-reply-to \
#		`#--dry-run` \
#		--to="linux-acpi@vger.kernel.org" \
#		--cc="linux-serial@vger.kernel.org" \
#		--cc="linux-kernel@vger.kernel.org" \
#		--cc="linux-arm-kernel@lists.infradead.org" \
#		--cc="Aleksey Makarov <aleksey.makarov@linaro.org>" \
#		--cc="Russell King <linux@arm.linux.org.uk>" \
#		--cc="Greg Kroah-Hartman <gregkh@linuxfoundation.org>" \
#		--cc="Rafael J. Wysocki <rjw@rjwysocki.net>" \
#		--cc="Leif Lindholm <leif.lindholm@linaro.org>" \
#		--cc="Graeme Gregory <graeme.gregory@linaro.org>" \
#		--cc="Al Stone <ahs3@redhat.com>" \
#		--cc="Christopher Covington <cov@codeaurora.org>" \
#		--cc='Matthias Brugger <mbrugger@suse.com>' \
#		outgoing/*

	# -------------------------------------------------------
	# reply with CC
	# -------------------------------------------------------

# git send-email --cc-cmd='./scripts/get_maintainer.pl --interactive --no-git-fallback' \
# 	--in-reply-to='20160815123934.GJ1041@n2100.armlinux.org.uk' \
# 	--to="Rafael J. Wysocki <rjw@rjwysocki.net>" \
# 	--to="Greg Kroah-Hartman <gregkh@linuxfoundation.org>" \
# 	--cc="linux-serial@vger.kernel.org" \
# 	--cc="linux-acpi@vger.kernel.org" \
# 	--cc="linux-kernel@vger.kernel.org" \
# 	--cc="linux-arm-kernel@lists.infradead.org" \
# 	--cc="Aleksey Makarov <aleksey.makarov@linaro.org>" \
# 	--cc="Russell King <linux@arm.linux.org.uk>" \
# 	--cc="Len Brown <lenb@kernel.org>" \
# 	--cc="Leif Lindholm <leif.lindholm@linaro.org>" \
# 	--cc="Graeme Gregory <graeme.gregory@linaro.org>" \
# 	--cc="Al Stone <ahs3@redhat.com>" \
# 	--cc="Christopher Covington <cov@codeaurora.org>" \
# 	--cc="Yury Norov <ynorov@caviumnetworks.com>" \
# 	--cc="Peter Hurley <peter@hurleysoftware.com>" \
# 	--cc="Andy Shevchenko <andy.shevchenko@gmail.com>" \
# 	--cc='"Zheng, Lv" <lv.zheng@intel.com>' \
# 	--cc="Mark Salter <msalter@redhat.com>" \
# 	--cc="Kefeng Wang <wangkefeng.wang@huawei.com>" \
# 	`#--dry-run` \
# 		outgoing/v9-0004-serial-pl011-add-console-matching-function.patch

	# -------------------------------------------------------
	# reply
	# -------------------------------------------------------

# git send-email --suppress-cc=all \
#	--in-reply-to=20160527171424.GG10909@e104818-lin.cambridge.arm.com \
#	--to="Catalin Marinas <catalin.marinas@arm.com>" \
#	--cc="Will Deacon <will.deacon@arm.com>" \
#	--cc="Rafael J. Wysocki <rjw@rjwysocki.net>" \
#	--cc="Len Brown <lenb@kernel.org>" \
#	--cc="linux-acpi@vger.kernel.org" \
#	--cc="linux-arm-kernel@lists.infradead.org" \
#	--cc="linux-kernel@vger.kernel.org" \
#	--cc="Aleksey Makarov <aleksey.makarov@linaro.org>" \
#	--cc="Graeme Gregory <graeme.gregory@linaro.org>" \
#	--cc="Jon Masters <jcm@redhat.com>" \
#	--cc='"Zheng, Lv" <lv.zheng@intel.com>' \
#	--cc="Mark Rutland <mark.rutland@arm.com>" \
#	`#--dry-run` \
#	`#--smtp-debug` \
#	outgoing/v3-0005-ACPI-ARM64-support-for-ACPI_TABLE_UPGRADE.patch

	# -------------------------------------------------------
	# store
	# -------------------------------------------------------


#		--to="Aleksey Makarov <aleksey.makarov@gmail.com>" \
# 		--to="Aleksey Makarov <aleksey.makarov@caviumnetworks.com>" \
#		--to="Aleksey Makarov <aleksey.makarov@auriga.com>" \
#		--to="Graeme Gregory <graeme.gregory@linaro.org>" \
#		--to="Robert Richter <robert.richter@caviumnetworks.com>" \
#		--to="David Daney <david.daney@cavium.com>" \
#
#		--cc="Aleksey Makarov <aleksey.makarov@auriga.org>" \
#		--cc="Aleksey Makarov <aleksey.makarov@linaro.org>" \
#		--cc="Aleksey Makarov <aleksey.makarov@caviumnetworks.com>" \
#		--cc="Aleksey Makarov <aleksey.makarov@gmail.com>" \
#		--cc="David Daney <david.daney@cavium.com>" \
#		--cc="Sunil Goutham <Sunil.Goutham@caviumnetworks.com>" \
#		--cc="t-kdev@caviumnetworks.com" \
#		--cc="thunderx_kdev@auriga.ru" \
#		--cc="Robert Richter <robert.richter@caviumnetworks.com>" \
#
#		--to="netdev@vger.kernel.org" \
#		--to="linux-mips@linux-mips.org" \
#		--to="linux-mmc@vger.kernel.org" \
#		--to="linux-ide@vger.kernel.org" \
#
#		--cc="linux-serial@vger.kernel.org" \
#		--cc="linux-mips@linux-mips.org" \
#
#		--cc="Alexey Klimov <klimov.linux@gmail.com>" \
#		--cc="David Daney <david.daney@cavium.com>" \
#		--cc="Robert Richter <robert.richter@caviumnetworks.com>" \
#		--cc="Sunil Goutham <Sunil.Goutham@caviumnetworks.com>" \
#		--cc="Leif Lindholm <leif.lindholm@linaro.org>" \

	true
}

# ----------------------------------------------------------------------------
# main
# ----------------------------------------------------------------------------

if [ $# != 0 ] ; then
	case $1 in

		"sf" )	submit_format	;;
		"ss" )	submit_submit	;;

		"cscope" )
			rm -f cscope.files cscope.out cscope.out.in cscope.out.po
			make cscope
			;;

		"sparse")
			shift
			make C=1 CF="-D__CHECK_ENDIAN__" Image
			# make V=1 C=1 CF="-D__CHECK_ENDIAN__" Image
			;;

		"disasm")
			shift
			${CROSS_COMPILE}objdump -afdS $1 # > ${1}.S
			;;

		"gdb")
			shift
			${CROSS_COMPILE}gdb ./vmlinux
			;;

		*)
			p=$1
			shift
			${CROSS_COMPILE}${p} $*
			;;

	esac

else
	. ./mk
fi

exit

# ----------------------------------------------------------------------------
# submit
# ----------------------------------------------------------------------------

# echo -n "${RED}continue?${RESET} "
# read
# if [ x$REPLY != x"yes" ] ; then
# 	echo "${RED}exiting...${RESET}"
# 	exit
# fi

# -------------------------------------------------------------------------------------------
# RTSoft -- smx6
# -------------------------------------------------------------------------------------------

# . /home/amakarov/work/kontron-samx/sdk/environment-setup-cortexa9hf-vfp-neon-poky-linux-gnueabi

# server_ip=192.168.11.185
# dtb=imx6dl-smx6-lcd.dtb
# dtb=imx6dl-smx6-lvds.dtb
# dtb=imx6q-smx6-lvds.dtb
#
# uploadx () {
# 	upload_src=$1
# 	upload_dst=$2
# 	upload_server=$3
# 	upload_login=$4
# 	upload_pwd=$5
# expect << EOF
# set timeout 1000
# spawn scp $upload_src ${upload_login}@${upload_server}:$upload_dst
# expect "assword: "
# send "${upload_pwd}\r\n"
# expect eof
# EOF
# }
#
# build_kernel () {
# 	make -j6 uImage LOADADDR=0x10008000
# }
# build_dtb    () {
# 	make ${dtb}
# 	make imx6q-smx6-lcd.dtb
# }
# build        () {
# 	build_kernel
# 	build_dtb
# }
#
# upload_kernel() {
# 	uploadx arch/arm/boot/uImage       /var/lib/tftpboot $server_ip root vrtx99
# }
# upload_dtb   () {
# 	uploadx arch/arm/boot/dts/${dtb}   /var/lib/tftpboot $server_ip root vrtx99
# }
# upload       () {
# 	upload_kernel
# 	upload_dtb
# }
#
# reboot () {
# 	snmpset -v 1 -c SWITCH 192.168.1.199 1.3.6.1.4.1.25728.5800.3.1.3.1 i 0
# 	sleep 3
# 	snmpset -v 1 -c SWITCH 192.168.1.199 1.3.6.1.4.1.25728.5800.3.1.3.1 i 1
# }
