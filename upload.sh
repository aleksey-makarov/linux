octeon_upload() {

	DTB=arch/mips/boot/dts/octeon_sff7000.dtb
	DTB_name=`basename ${DTB}`
	DTB_address=0x30000000
	DTB_size=0x10000

	if grep -q "^CONFIG_CPU_BIG_ENDIAN=y" .config ; then
		echo_log "big endian"
		# rootpartition="LABEL=root_big"
		rootpartition="/dev/sda1"
	else
		echo_log "little endian"
		# rootpartition="LABEL=root_little"
		rootpartition="/dev/sda2"
	fi

	(
		echo namedalloc fdt ${DTB_size} ${DTB_address}
		echo tftp     ${DTB_address} ${DTB_name}
		echo fdt addr ${DTB_address}
		echo tftp     0x20000000 vmlinux
		echo bootoctlinux 0x20000000 numcores=4 endbootargs mem=0 root=${rootpartition} init=/sbin/init loglevel=8
	) > cmd

	sudo cp ${DTB} /srv/tftp
	sudo ${CROSS_COMPILE}strip ./vmlinux -o /srv/tftp/vmlinux
	sudo mkimage -A mips -O linux -T script -C none -a 0 -e 0 -n "autoscr example script" -d ./cmd /srv/tftp/cmd
}

upload() {

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

	# sudo cp arch/arm64/boot/dts/cavium/thunder-88xx.dtb	/srv/tftp/efi

	# cp arch/arm/boot/zImage /srv/tftp
	# echo " done"

  echo -n "uploading..."
  pbzip2 -c arch/arm64/boot/Image | ssh amakarov@build 'pbzip2 -dc > /srv/tftp/Image'
  echo " done"

	# true
}

upload