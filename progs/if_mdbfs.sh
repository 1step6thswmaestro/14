rm disk.img; dd if=/dev/zero of=disk.img count=2 bs=1M;
make clean;
make mdbfs;
./mdbfs;
