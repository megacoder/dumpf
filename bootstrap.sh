#!/bin/sh
# vim: nonu ts=8 sw=8
(
	exec 2>&1
	autoreconf -fvim
	./configure
	make dist
	rm -rf RPM
	mkdir -p RPM/{SOURCES,RPMS,SRPMS,BUILD,SPECS}
	markdown2 README.md | tee README.html | lynx -dump >README
	rpmbuild							\
		-D"_topdir ${PWD}/RPM"					\
		-D"_sourcedir ${PWD}"					\
		-D"_specdir ${PWD}"					\
		-ba							\
		dumpf.spec
) | tee bootstrap.log
