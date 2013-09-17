#!/bin/sh
autoreconf -fvim
./configure
make dist
cp *gz ~/rpm/SOURCES
rpmbuild -ba *.spec
