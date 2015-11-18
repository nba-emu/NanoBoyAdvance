#/bin/sh

cd nanoboyadvance
make distclean && make
cd ..
ln -s nanoboyadvance/nanoboyadvance nba
