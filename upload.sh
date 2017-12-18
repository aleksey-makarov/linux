source ./setenv.sh

upload_modules_alarm() {
	KERNEL_RELEASE=`cat include/config/kernel.release`
	echo_log "uploading modules (${KERNEL_RELEASE}) ..."
	tar -C ${INSTALL_MOD_PATH}/lib/modules -cf ${KERNEL_RELEASE}.tar ${KERNEL_RELEASE}
	pbzip2 -c ${KERNEL_RELEASE}.tar | ssh alarm 'cd /lib/modules ; pbzip2 -dc | sudo tar x'
	echo_log "done"
}

upload_ptp_alarm() {
	PTP_MODULE=drivers/net/ethernet/cavium/common/cavium_ptp.ko
	PTP_MODULE_BASENAME=`basename ${PTP_MODULE}`
	echo_log "uploading module (${PTP_MODULE_BASENAME}) ..."
	# pbzip2 -c ${PTP_MODULE} | ssh alarm "sudo pbzip2 -dc > ~root/${PTP_MODULE_BASENAME}"
	pbzip2 -c ${PTP_MODULE} | ssh alarm "pbzip2 -dc | sudo tee ~root/${PTP_MODULE_BASENAME} > /dev/null"
	echo_log "done"
}

upload() {
	echo_log "uploading..."
	pbzip2 -c arch/arm64/boot/Image | ssh amakarov@build 'pbzip2 -dc > /srv/tftp/Image'
	echo_log "done"
}

upload
# upload_ptp_alarm
# upload_modules_alarm

exit

# echo -n "GRUB crb-2s: start uploading..."
# # xz -zc arch/arm64/boot/Image | ssh root@172.27.65.27 'xzcat > /boot/Image.test'
# xz -zc arch/arm64/boot/Image | ssh root@172.27.64.243 'xzcat > /boot/Image.test'
# echo " done"

# outfilename=zImage
# xz -zc arch/arm64/boot/Image | ssh aleksey.makarov@lab.validation.linaro.org "xzcat > /srv/tftp/cavium/$outfilename"
# echo -n "TFTP: crb-1s: start uploading as $outfilename ..."
# xz -zc arch/arm64/boot/Image | ssh root@172.27.65.24 'xzcat > /boot/efi/EFI/debian/Image.debug'
# xz -zc arch/arm64/boot/Image | ssh aleksey.makarov@linaro "xzcat > /srv/tftp/cavium/$outfilename"
# xz -zc arch/arm64/boot/Image | ssh aleksey.makarov@lab.validation.linaro.org "xzcat > /srv/tftp/cavium/$outfilename"

# echo -n "GRUB: start uploading..."
# xz -zc arch/arm64/boot/Image | ssh root@172.27.65.24 'xzcat > /mnt/sda2/boot/override/Image'
# echo " done"

# octeon_upload() {
# 
# 	DTB=arch/mips/boot/dts/octeon_sff7000.dtb
# 	DTB_name=`basename ${DTB}`
# 	DTB_address=0x30000000
# 	DTB_size=0x10000
# 
# 	if grep -q "^CONFIG_CPU_BIG_ENDIAN=y" .config ; then
# 		echo_log "big endian"
# 		# rootpartition="LABEL=root_big"
# 		rootpartition="/dev/sda1"
# 	else
# 		echo_log "little endian"
# 		# rootpartition="LABEL=root_little"
# 		rootpartition="/dev/sda2"
# 	fi
# 
# 	(
# 		echo namedalloc fdt ${DTB_size} ${DTB_address}
# 		echo tftp     ${DTB_address} ${DTB_name}
# 		echo fdt addr ${DTB_address}
# 		echo tftp     0x20000000 vmlinux
# 		echo bootoctlinux 0x20000000 numcores=4 endbootargs mem=0 root=${rootpartition} init=/sbin/init loglevel=8
# 	) > cmd
# 
# 	sudo cp ${DTB} /srv/tftp
# 	sudo ${CROSS_COMPILE}strip ./vmlinux -o /srv/tftp/vmlinux
# 	sudo mkimage -A mips -O linux -T script -C none -a 0 -e 0 -n "autoscr example script" -d ./cmd /srv/tftp/cmd
# }
