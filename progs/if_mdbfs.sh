rm disk.img; dd if=/dev/zero of=disk.img count=3 bs=1M;
make clean;
make mdbfs;
./mdbfs;
