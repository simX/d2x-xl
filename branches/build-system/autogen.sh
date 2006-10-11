#!/bin/sh
set -e
echo "If you get errors, you may need newer versions of automake and autoconf."
echo "You'll need at least automake 1.7 and autoconf 2.50."

WANT_AUTOMAKE=1.7 aclocal $ACLOCAL_FLAGS
autoheader
automake --add-missing --copy
autoconf

#./configure "$@"
echo "Now you are ready to run ./configure"
