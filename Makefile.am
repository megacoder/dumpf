# vim: noet sw=8 ts=8 nonu

SUBDIRS	= src

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL LICENSE NEWS README.md	\
	TODO dumpf.1

man_MANS = dumpf.1

README README.html: README.md
	markdown2 $^ | tee README.html | lynx -dump >README
