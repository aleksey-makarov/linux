. ./setenv.sh

echo_log `which ${CROSS_COMPILE}gcc`

# make Image
# make V=1
make -j${P} Image
# make -j${P} modules
# make modules
./upload.sh
