#!/bin/sh

echo ""
echo "building jemalloc..."
DIR=`pwd`
cd deps/jemalloc-3.3.1
if [ ! -f Makefile ]; then
	./configure
   	make
fi
cd $DIR
echo "building jemalloc finished"
echo ""


echo "#ifndef SSDB_VERSION" > src/version.h
echo "#define SSDB_VERSION \"`cat version`\"" >> src/version.h
echo "#endif" >> src/version.h

TARGET_OS=`uname -s`
case "$TARGET_OS" in
    Darwin)
        PLATFORM="macosx"
        PLATFORM_LDFLAGS="-pthread"
        ;;
    Linux)
        PLATFORM="linux"
        PLATFORM_LDFLAGS="-pthread"
        ;;
    CYGWIN_*)
        PLATFORM="mingw"
        PLATFORM_LDFLAGS="-pthread"
        ;;
    SunOS)
        PLATFORM="solaris"
        PLATFORM_LIBS="-lpthread -lrt"
        ;;
    FreeBSD)
        PLATFORM="freebsd"
        PLATFORM_LIBS="-lpthread"
        ;;
    NetBSD)
        PLATFORM="bsd"
        PLATFORM_LIBS="-lpthread -lgcc_s"
        ;;
    OpenBSD)
        PLATFORM="bsd"
        PLATFORM_LDFLAGS="-pthread"
        ;;
    DragonFly)
        PLATFORM="freebsd"
        PLATFORM_LIBS="-lpthread"
        ;;
    OS_ANDROID_CROSSCOMPILE)
        PLATFORM="generic"
        PLATFORM_LDFLAGS=""  # All pthread features are in the Android C library
        ;;
    HP-UX)
        PLATFORM="generic"
        PLATFORM_LDFLAGS="-pthread"
        ;;
    *)
        echo "Unknown platform!" >&2
        exit 1
esac

rm -f build_config.mk
echo "LEVELDB=leveldb-1.9.0" >> build_config.mk
echo "LUA=lua-5.2.2" >> build_config.mk
echo "PLATFORM=$PLATFORM" >> build_config.mk
echo "PLATFORM_LDFLAGS=$PLATFORM_LDFLAGS" >> build_config.mk

