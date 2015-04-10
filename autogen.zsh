#!/bin/zsh

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile ${PWD:t:r}"
	echo "Install the appropriate package for your distribution,"
	echo "or get the source tarball at http://ftp.gnu.org/gnu/autoconf/"
	DIE=1
}

(automake --version) </dev/null >/dev/null 2>&1 ||	{
	echo
	echo "You must have automake installed to compile ${PWD:t:r}"
	echo "Install the appropriate package for your distribution,"
	echo "or get the source tarball at http://ftp.gnu.org/gnu/automake/"
	DIE=1
}

(aclocal --version) </dev/null >/dev/null 2>&1			||	{
	echo
	echo "You must have aclocal installed to compile ${PWD:t:r}"
	echo "Install the appropriate package for your distribution,"
	echo "or get the source tarball at http://ftp.gnu.org/gnu/aclocal/"
	DIE=1
}

if test "$DIE" -eq 1; then
	exit 1
fi

rm -rf autom4te.cache configure

aclocal ${ACLOCAL_FLAGS}	|| exit $?
autoheader -Wall -Werror	|| exit $?
automake --add-missing		|| exit $?
autoconf			|| exit $?
