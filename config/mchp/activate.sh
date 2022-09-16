#!/bin/sh
source ~/.profile

if xc32-gcc --version; then
    echo "Success"
else
    echo "Unable to find Microchip xc32 compiler, exit status: $?"
fi

ln -s ../../build build
ln -s ../../build_overrides build_overrides
ln -s ../../config config
ln -s ../../scripts scripts
ln -s ../../third_party third_party
ln -s ../../src src
ln -s ../../zzz_generated zzz_generated

