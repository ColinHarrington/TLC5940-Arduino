#!/bin/sh
# Run this script to package the Tlc5940 library for distribution
# Alex Leone, 2009-05-07

VERSION=014

rm -rf ../Tlc5940
rm -rf ../Tlc5940_r*.zip
cp -r Tlc5940 ../
cd ..
rm -rf Tlc5940/Tlc5940.o
find Tlc5940/ -name ".svn" -type d | xargs rm -rf
find Tlc5940/ -name "applet" -type d | xargs rm -rf
zip -r Tlc5940_r$VERSION.zip Tlc5940
cd Tlc5940
#doxygen
