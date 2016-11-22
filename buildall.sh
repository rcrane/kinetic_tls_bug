#!/bin/bash

set -e
set -x

OPENSSLCFLAG="-O3 -fPIC -DOPENSSL_API_COMPAT=0x10100000L"

if [ ! -d openssl-gcc ] ; then
    git clone -b OpenSSL_1_0_2-stable https://github.com/openssl/openssl.git openssl-gcc
    pushd openssl-gcc
    if [ ! -d target ] ; then
	mkdir target
    fi
    ./Configure --prefix=$(readlink -f target) linux-x86_64
    sed -i '/^CFLAG=/ s/$/ '"$OPENSSLCFLAG"'/' Makefile
    make depend -j6
    make -j6
    make install_sw
    popd
fi

SSL=$(readlink -f openssl-gcc/target)
# -DKINETIC_USE_TLS_1_2 
KINETICCFLAGS="-std=c99 -O3 -fPIC -D_XOPEN_SOURCE=600"

if [ ! -d kinetic-c-gcc ] ; then

    git clone --recursive https://github.com/Kinetic/kinetic-c.git kinetic-c-gcc

    cp -a kinetic-c-gcc kinetic-c-patched
    cp send_helper.fixed kinetic-c-patched/src/lib/bus/send_helper.c

    pushd kinetic-c-gcc
    	pushd vendor/json-c
    		./autogen.sh CFLAGS="$KINETICCFLAGS"
	    	make CFLAGS="$KINETICCFLAGS" -j5
       	popd
	export OPENSSL_PATH=$SSL
	make CFLAGS="$KINETICCFLAGS" -j5

    popd


    pushd kinetic-c-patched
        pushd vendor/json-c
            ./autogen.sh CFLAGS="$KINETICCFLAGS"
            make CFLAGS="$KINETICCFLAGS" -j5
        popd
    make CFLAGS="$KINETICCFLAGS" -j5
    popd

fi

make
