./rmmod_mfs.sh
./progs/umount.sh
sudo make clean
make
sudo insmod mfs_mod.ko
cd progs
rm -f disk.img
dd if=/dev/zero of=disk.img count=3 bs=1M
sudo make clean
make mdbfs
./mdbfs
