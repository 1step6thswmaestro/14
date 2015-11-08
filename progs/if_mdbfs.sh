rm asdf*;
rm disk.img; dd if=/dev/zero of=disk.img count=1 bs=1M;
make clean;
make mdbfs;
./mdbfs;
