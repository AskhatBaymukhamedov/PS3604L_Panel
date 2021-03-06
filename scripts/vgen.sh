#!/bin/sh

FILE="$1"
if [ ! -f $FILE ]; then
    echo "File $FILE does not exist"
    set -e
    exit 1
fi

sed -i 's/^\(.*fwinfoBuild.*= \)\([0-9]\+\);/echo "\1$((\2+1));"/ge' $FILE

nmajor=`sed -n 's/.*fwinfoMajor.*= \([0-9]\+\);/\1/p' $FILE`
nminor=`sed -n 's/.*fwinfoMinor.*= \([0-9]\+\);/\1/p' $FILE`
nrev=`sed -n 's/.*fwinfoRevision.*= \([0-9]\+\);/\1/p' $FILE`
nbuild=`sed -n 's/.*fwinfoBuild.*= \([0-9]\+\);/\1/p' $FILE`

export info=\"$nmajor.$nminor.$nrev.$nbuild\"
sed -i 's/\(.*fwinfoVersion\[\].*= \){"\(.*\)"}/echo \1{$info}\\/ge' $FILE

export date=\"$(date -u +%Y-%m-%d)\"
sed -i 's/\(.*fwinfoDate\[\].*= \){"\(.*\)"}/echo \1{$date}\\/ge' $FILE

export time=\"$(date -u +%H:%M:%S)\"
sed -i 's/\(.*fwinfoTime\[\].*= \){"\(.*\)"}/echo \1{$time}\\/ge' $FILE

echo version:  $nmajor.$nminor.$nrev.$nbuild
