PREFIX=/usr/local/ssdb

$(shell sh build.sh 1>&2)
include build_config.mk

all:
	chmod u+x deps/${LEVELDB}/build_detect_platform
	chmod u+x deps/cpy/cpy
	chmod u+x tools/ssdb-cli tools/ssdb-benchmark

	@echo "\n---> Building LevelDB\n"
	cd deps/${LEVELDB}; make

	@echo "\n---> Building Lua\n"
	cd deps/${LUA}; make ${PLATFORM}

	@echo "\n---> Building SSDB\n"
	$(eval export CFLAGS=-DNDEBUG -Wall -O2 -Wno-sign-compare)
	cd src/util; make
	cd src; make
	cd tools; make

install:
	mkdir -p ${PREFIX}
	mkdir -p ${PREFIX}/deps
	mkdir -p ${PREFIX}/var
	mkdir -p ${PREFIX}/var_slave
	cp ssdb-server ssdb.conf ssdb_slave.conf ${PREFIX}
	cp -r api ${PREFIX}
	cp -r tools/* ${PREFIX}
	cp -r deps/cpy ${PREFIX}/deps
	chmod ugo+rwx ${PREFIX}
	rm -f ${PREFIX}/Makefile

clean:
	rm -rf  *.exe.stackdump  api/cpy/_cpy_  api/python/SSDB.pyc  db_test
	cd deps/cpy; make clean
	cd deps/${LEVELDB}; make clean
	cd deps/${LUA}; make clean
	cd src/util; make clean
	cd src; make clean
	cd tools; make clean


