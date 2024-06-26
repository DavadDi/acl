SHELL = /bin/sh
#CC      = g++
CC	= ${ENV_CPP}
AR      = ${ENV_AR}
ARFL    = rv
#ARFL    = cru
RANLIB  = ${ENV_RANLIB}

FLAGS   = ${ENV_FLAGS}

CFLAGS = -c -g -W \
-fPIC \
-Wall \
-Werror \
-Wpointer-arith \
-D_REENTRANT \
-D_POSIX_PTHREAD_SEMANTICS \
-Wno-long-long \
-Wformat \
-DHAS_MYSQL_DLL \
-DHAS_PGSQL_DLL \
-DHAS_SQLITE_DLL \
-Winvalid-pch \
-DACL_PREPARE_COMPILE \
#-DHAS_ZLIB_DLL \
#-DACL_CLIENT_ONLY \
#-DACL_REDIS_DISABLE \
#-DACL_BEANSTALK_DISABLE \
#-DACL_DB_DISABLE \
#-DACL_MIME_DISABLE \
#-DUSE_DYNAMIC
# -Wcast-align
#-Wcast-qual
#just for freebsd's iconv.h
CFLAGS += -I/usr/local/include

shared_ldflags = -lprotocol -lacl -lz -lpthread

STATIC_LIBNAME = libacl_cpp.a
SHARED_LIBNAME = libacl_cpp.so

###########################################################
#Check system:
#       Linux, SunOS, Solaris, BSD variants, AIX, HP-UX
SYSLIB =
CHECKSYSRES = @echo "Unknow system type!";exit 1
UNIXNAME = $(shell uname -sm)
UNIXTYPE = LINUX
polarssl =

ifeq ($(CC),)
        CC = g++
endif

ifeq ($(findstring g++, $(CC)), g++)
	GCC_VERSION:=$(shell echo `gcc --version|grep ^gcc|cut -d' ' -f3`)
	GCC_MAJOR:=$(shell echo "$(GCC_VERSION)" | cut -d'.' -f1)
	GCC_MINOR:=$(shell echo "$(GCC_VERSION)" | cut -d'.' -f2)
	GCC_SUB:=$(shell echo "$(GCC_VERSION)" | cut -d'.' -f3)
	GCC_VER:=$(shell [ $(GCC_MAJOR) -gt 4 -o \( $(GCC_MAJOR) -eq 4 -a $(GCC_MINOR) -ge 4 \) ] && echo true)
	ifeq ($(GCC_VER), true)
		CFLAGS += -Wno-implicit-fallthrough
	endif
endif

ifeq ($(AR),)
	AR = ar
endif

ifeq ($(RANLIB),)
	RANLIB = ranlib
endif

ifeq ($(findstring clang++, $(CC)), clang++)
	CFLAGS += -Wno-invalid-source-encoding \
		  -Wno-invalid-offsetof
endif

ifeq ($(findstring cosmoc+, $(CC)), cosmoc+)
	CFLAGS += -DCOSMOCC
	CFLAGS += -Wno-implicit-fallthrough
	CFLAGS += -DLINUX2
	CFLAGS += -fexceptions
	AR = cosmoar

endif

# For FreeBSD
ifeq ($(findstring FreeBSD, $(UNIXNAME)), FreeBSD)
	UNIXTYPE = FREEBSD
	CFLAGS += -Wshadow
	CFLAGS += -O3

	ifeq ($(OPENSSL_STATIC), yes)
		CFLAGS += -DHAS_OPENSSL
	else
		CFLAGS += -DHAS_OPENSSL_DLL
	endif

	CFLAGS += -I../include/openssl-1.1.1q

	ifeq ($(POLARSSL_STATIC), yes)
		CFLAGS += -DHAS_POLARSSL
	else
		CFLAGS += -DHAS_POLARSSL_DLL
	endif

	ifeq ($(MBEDTLS_STATIC), yes)
		CFLAGS += -DHAS_MBEDTLS
	else
		CFLAGS += -DHAS_MBEDTLS_DLL
	endif

	shared_ldflags = -lprotocol -lacl -L/usr/local/lib -liconv -lz -lpthread
endif

# For Darwin
ifeq ($(findstring Darwin, $(UNIXNAME)), Darwin)
#	CC += -arch x86_64 -arch arm64
	CFLAGS += -Wno-invalid-source-encoding \
		  -Wno-invalid-offsetof \
		  -Wno-unused-private-field \
		  -fdata-sections \
		  -ffunction-sections \
		  -flto \
		  -Os
	CFLAGS += -Wshadow
	UNIXTYPE = MACOSX

	ifeq ($(OPENSSL_STATIC), yes)
		CFLAGS += -DHAS_OPENSSL
	else
		CFLAGS += -DHAS_OPENSSL_DLL
	endif

	CFLAGS += -I../include/openssl-1.1.1q

	ifeq ($(POLARSSL_STATIC), yes)
		CFLAGS += -DHAS_POLARSSL
	else
		CFLAGS += -DHAS_POLARSSL_DLL
	endif
	ifeq ($(MBEDTLS_STATIC), yes)
		CFLAGS += -DHAS_MBEDTLS
	else
		CFLAGS += -DHAS_MBEDTLS_DLL
	endif

	shared_ldflags = -lprotocol -lacl -L/usr/lib -liconv -lz -lpthread
	SHARED_LIBNAME = libacl_cpp.dylib
endif

# For Linux
ifeq ($(findstring Linux, $(UNIXNAME)), Linux)
	UNIXTYPE = LINUX
	CFLAGS += -Wshadow
	CFLAGS += -O3

#	When running "$make OPENSSL_STATIC=yes", the openssl's libs will be
#	linked in static mode, or the libs will be loaded in dynamic way.

	ifeq ($(OPENSSL_STATIC), yes)
		CFLAGS += -DHAS_OPENSSL
	else
		CFLAGS += -DHAS_OPENSSL_DLL
	endif
	CFLAGS += -I../include/openssl-1.1.1q

	ifeq ($(POLARSSL_STATIC), yes)
		CFLAGS += -DHAS_POLARSSL
	else
		CFLAGS += -DHAS_POLARSSL_DLL
	endif

	ifneq ($(findstring cosmoc+, $(CC)), cosmoc+)
		ifeq ($(MBEDTLS_STATIC), yes)
			CFLAGS += -DHAS_MBEDTLS
		else
			CFLAGS += -DHAS_MBEDTLS_DLL
		endif
	endif

	shared_ldflags = -lprotocol -lacl -lrt -lpthread -lz -ldl
endif

# For CYGWIN
ifeq ($(findstring CYGWIN, $(UNIXNAME)), CYGWIN)
	CFLAGS += -DLINUX2 -DMINGW
	CFLAGS += -Wshadow
	CFLAGS += -O3
	UNIXTYPE = LINUX
endif

# For MINGW
ifeq ($(findstring MINGW, $(UNIXNAME)), MINGW)
	CFLAGS += -DLINUX2 -DMINGW
	CFLAGS += -Wshadow
	CFLAGS += -O3
	UNIXTYPE = LINUX
endif

# For MSYS
ifeq ($(findstring MSYS, $(UNIXNAME)), MSYS)
	CFLAGS += -DLINUX2 -DMINGW
	CFLAGS += -Wshadow
	CFLAGS += -O3
	UNIXTYPE = LINUX
endif

# For SunOS
ifeq ($(findstring SunOS, $(UNIXNAME)), SunOS)
	ifeq ($(findstring 86, $(UNIXNAME)), 86)
		SYSLIB = -lsocket -lnsl -lrt
	endif
	ifeq ($(findstring sun4u, $(UNIXNAME)), sun4u)
		SYSLIB = -lsocket -lnsl -lrt
	endif
	CFLAGS += -DSUNOS5
	CFLAGS += -O3
	UNIXTYPE = SUNOS5
endif

# For HP-UX
ifeq ($(findstring HP-UX, $(UNIXNAME)), HP-UX)
	CFLAGS += -DHP_UX -DHPUX11
	CFLAGS += -Wshadow
	CFLAGS += -O3
	UNIXTYPE = HPUX
endif

ifeq ($(findstring HAS_POLARSSL, $(FLAGS)), HAS_POLARSSL)
	CFLAGS += -DHAS_POLARSSL
	polarssl = on
endif

ifeq ($(findstring on, $(polarssl)), on)
	CFLAGS += -DHAS_POLARSSL
endif

#Find system type.
ifneq ($(SYSPATH),)
	CHECKSYSRES = @echo "System is $(shell uname -sm)"
endif
###########################################################

OBJ_PATH_DST = ./Debug
LIB_PATH_DST = ./lib
DEF_PATH_DST = ./Debug

CFLAGS += -I./src -I./include -I../lib_acl/include -I../lib_protocol/include \
	  -I../include/sqlite -I../include/mysql -I../include \
	  -I../include/pgsql \
	  -I../include/mbedtls/3.3.0

#Project's objs
OBJS_DST = $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/stdlib/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/stream/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/master/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/ipc/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/db/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/event/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/queue/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/http/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/hsocket/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/beanstalk/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/memcache/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/session/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/smtp/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/mime/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/mime/internal/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/net/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/connpool/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/redis/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/disque/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/serialize/*.cpp))) \
	   $(patsubst %.cpp, $(OBJ_PATH_DST)/%.o, $(notdir $(wildcard src/mqtt/*.cpp)))

OBJS_DEF = $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/stdlib/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/stream/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/master/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/ipc/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/db/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/event/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/queue/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/http/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/hsocket/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/beanstalk/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/memcache/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/session/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/smtp/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/mime/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/mime/internal/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/net/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/connpool/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/redis/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/disque/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/serialize/*.cpp))) \
	   $(patsubst %.cpp, $(DEF_PATH_DST)/%.inc, $(notdir $(wildcard src/mqtt/*.cpp)))

###########################################################
.PHONY = static shared clean
VERSION = 1.1.1.2
DATE_NOW = 20`date +%y`.`date +%m`.`date +%d`

PCH = src/acl_stdafx.hpp.gch

lib: static
all: static shared sample

$(shell mkdir -p $(DEF_PATH_DST))
#ifneq ($(MAKECMDGOALS),clean)
#-include $(OBJS_DEF)
#endif

COMPILE = $(CC) $(CFLAGS)
COMPILE_OBJ = @(echo 'building $<'; $(COMPILE) $< -o $@)
CREATE_DEF = @(echo 'creating $@'; rm -f $@; \
	$(COMPILE) -MM $< > $@.$$$$; \
	sed 's,.*.o\( \)*:,$(patsubst %.inc,%.o,$@) $@ :,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$)

###########################################################

sample: static
	@(cd samples; make)

#static: depends $(OBJS_DST)
static: $(OBJS_DST)
	@echo 'creating $(LIB_PATH_DST)/$(STATIC_LIBNAME)'
	@$(AR) $(ARFL) $(LIB_PATH_DST)/$(STATIC_LIBNAME) $(OBJS_DST)
	@$(RANLIB) $(LIB_PATH_DST)/$(STATIC_LIBNAME)
	@(cd $(LIB_PATH_DST); rm -f lib_acl_cpp.a; \
		ln -s $(STATIC_LIBNAME) lib_acl_cpp.a)
	@echo 'create $(LIB_PATH_DST)/$(STATIC_LIBNAME) ok!'

#shared: depends pch $(OBJS_DST)
shared: $(OBJS_DST)
	@echo 'creating $(SHARED_LIBNAME)'
	@if test -n "$(rpath)" && test "$(UNIXTYPE)" = "LINUX"; then \
		echo "building for linux"; \
		$(CC) -shared -o $(rpath)/$(SHARED_LIBNAME) $(OBJS_DST) \
			-L$(rpath) $(shared_ldflags) -Wl,-rpath,$(rpath); \
		echo 'build $(rpath)/$(SHARED_LIBNAME) ok!'; \
	elif test -n "$(rpath)" && test "$(UNIXTYPE)" = "SUNOS5"; then \
		echo "building for sunos5"; \
		$(CC) -shared -o $(rpath)/$(SHARED_LIBNAME) $(OBJS_DST) \
			-R$(rpath) -L$(rpath) $(shared_ldflags); \
		echo 'build $(rpath)/$(SHARED_LIBNAME) ok!'; \
	elif test -n "$(rpath)" && test "$(UNIXTYPE)" = "MACOSX"; then \
		echo "building for Darwin"; \
		$(CC) -shared -o $(rpath)/$(SHARED_LIBNAME) $(OBJS_DST) \
			-R$(rpath) -L$(rpath) $(shared_ldflags); \
		echo 'build $(rpath)/$(SHARED_LIBNAME) ok!'; \
	elif test -n "$(rpath)" && test "$(UNIXTYPE)" = "FREEBSD"; then \
		echo "building for FreeBSD"; \
		$(CC) -shared -o $(rpath)/$(SHARED_LIBNAME) $(OBJS_DST) \
			-R$(rpath) -L$(rpath) $(shared_ldflags); \
		echo 'build $(rpath)/$(SHARED_LIBNAME) ok!'; \
	else \
		echo 'skip build $(SHARED_LIBNAME); usage: make shared rpath=xxx'; \
	fi

depends: $(OBJS_DEF)
check:
	@if test "$(polarssl)" = "on"; then \
		rm -f Debug/polarssl_io.o; \
		rm -f Debug/polarssl_conf.o; \
		rm -f Debug/polarssl_io.inc; \
		rm -f Debug/polarssl_conf.inc; \
	fi

pch: src/acl_stdafx.hpp
	rm -f $(PCH)
	@if test "$(UNIXTYPE)" = "LINUX" || test "$(UNIXTYPE)" = "FREEBSD" \
		|| test "$(UNIXTYPE)" = "MACOSX"; then \
		$(CC) -o $(PCH) -x c++-header $(CFLAGS) src/acl_stdafx.hpp; \
	fi

# src
$(OBJ_PATH_DST)/%.o: ./src/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/stdlib/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/stream/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/master/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/ipc/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/db/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/event/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/queue/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/http/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/hsocket/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/beanstalk/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/memcache/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/session/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/connpool/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/smtp/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/mime/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/mime/internal/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/net/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/redis/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/disque/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/serialize/%.cpp
	$(COMPILE_OBJ)
$(OBJ_PATH_DST)/%.o: ./src/mqtt/%.cpp
	$(COMPILE_OBJ)

$(DEF_PATH_DST)/%.inc: ./src/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/stdlib/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/stream/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/master/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/ipc/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/db/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/event/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/queue/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/http/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/hsocket/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/beanstalk/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/memcache/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/session/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/connpool/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/smtp/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/mime/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/mime/internal/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/redis/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/disque/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/serialize/%.cpp
	$(CREATE_DEF)
$(DEF_PATH_DST)/%.inc: ./src/mqtt/%.cpp
	$(CREATE_DEF)

clean cl:
	rm -f $(LIB_PATH_DST)/libacl_cpp.a
	rm -f $(LIB_PATH_DST)/lib_acl_cpp.a
	rm -f $(LIB_PATH_DST)/libacl_cpp.so
	rm -f $(LIB_PATH_DST)/lib_acl_cpp.so
	rm -f $(OBJS_DST)
	rm -f $(OBJS_DEF)
	rm -f $(PCH)
	@(cd samples; make clean)

rebuild rb: clean static

build_src: clean
	@(echo "begin building src release...")
	@(rm -rf acl_cpp)
	@(echo "copy files ...")
	@(mkdir acl_cpp)
	@(cp -R src acl_cpp/)
	@(cp -R mime acl_cpp/)
	@(cp -R lib acl_cpp/)
	@(cp -R include acl_cpp/)
	@(cp -R samples acl_cpp/)
	@(cp -R Debug acl_cpp/)
	@(cp -R DebugDll acl_cpp/)
	@(cp -R Release acl_cpp/)
	@(cp -R ReleaseDll acl_cpp/)
	@(cp -R bin acl_cpp/)
	@(cp -R doc acl_cpp/)
	@(cp lib_acl_cpp_vc2003.vcproj acl_cpp/)
	@(cp acl_wrap_vc2003.sln acl_cpp/)
	@(cp lib_acl_cpp_vc2010.vcxproj acl_cpp/)
	@(cp acl_wrap_vc2010.sln acl_cpp/)
	@(cp Makefile acl_cpp/)
	@(cp changes.txt acl_cpp/)
	@(cp ReadMe.txt acl_cpp/)
	@(cp Doxyfile acl_cpp/)
	@(echo "make tar package ...")
	@(tar -cf acl_cpp.src.tar acl_cpp)
	@(echo "make gzip package ...")
	@(gzip -c acl_cpp.src.tar > acl_cpp.src.tgz)
	@(rm acl_cpp.src.tar)
	@(rm -rf acl_cpp)
	@(echo "move acl_cpp.src.tgz to ../acl_cpp$(VERSION).src.$(DATE_NOW).tgz")
	(mv acl_cpp.src.tgz ../acl_cpp$(VERSION).src.$(DATE_NOW).tgz)
