#!/bin/sh

mkdir -p lang/
mkdir -p lang/po/
cd src
xgettext --force-po -c -C *.cpp -o ../lang/po/messages.po
xgettext --force-po -c -C *.cpp -o ../lang/po/en.po
cd ..

mkdir -p lang/auto/

cd src
xgettext --force-po -c -C *.cpp -o ../lang/auto/messages.po
xgettext --force-po -c -C *.cpp -o ../lang/auto/en.po
cd ..

cp -r lang/* data/lang/
