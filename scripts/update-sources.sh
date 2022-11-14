#!/bin/bash

wd=$(pwd)

( cd "$wd/qlat-utils/pylib/cqlat_utils" ; bash update.sh )

( cd "$wd/qlat/pylib/cqlat" ; bash update.sh )

( cd "$wd/qlat-grid/pylib/cqlat_grid" ; bash update.sh )

(

distfiles="$wd/distfiles"

mkdir -p "$distfiles"
cd "$distfiles"

sha256sum *.tar.* | sort > sha256sums.txt

echo >> sha256sums.txt

sha256sum python-packages/*.* | sort >> sha256sums.txt

echo >> sha256sums.txt

for fn in * ; do
    if [ -e "$fn"/.git ] ; then
        echo -n "$fn: "
        ( cd "$fn" ; git rev-parse HEAD )
    fi
done | sort >> sha256sums.txt

cat sha256sums.txt

)