#! /usr/bin/env bash
#

#This scripts is a modified version of the buildall.script. 
#Thi script compiles and install all the minisip libraries and GTK GUI
#  for a win32 system.
#Modify the parameters (basically the prefix_*) to adapt to your cross-compile environment.
#It is not really good if you are developing or debugging, as everytime you run the script it 
#  recompiles ALL the source code.

#This options are added for the w32 compilation ... modify if needed, specially the cross-compile folder
#This is the CROSS_COMPILE_FOLDER
prefix_cross=/minisip
prefix_cross_include=$prefix_cross/include
prefix_cross_lib=$prefix_cross/lib
host_type_name="i586-mingw32msvc"
host_option="--host=$host_type_name"
build_option="--build=i686-pc-linux-gnu"
strip_bin="i586-mingw32msvc-strip"
#strip_generated_files="yes"

# Simple script to build minisip.
#
# This script needs to be executed at the root of the repository trunk.  It
# will build minisip and with all the associated libraries.  The build will be
# done "in place", that is, no files will be installed on your system.
#
# Instructions: 
#  Use buildall.sh script the first time you get the sources of minisip. It will
# create all files needed and compile all the sources. 
# Then, if you modify a file in the sources and do not want to recompile all 
# MiniSIP again, use the "buildonly.sh" script

# Note that you con modify the configuration of minisip by editing the 
# configure_params variable in this script. Some useful options are commented
# out by default, but feel free to use them.

SUBDIRS="${SUBDIRS} libmutil"
SUBDIRS="${SUBDIRS} libmnetutil"
SUBDIRS="${SUBDIRS} libmikey"
SUBDIRS="${SUBDIRS} libmsip"
#SUBDIRS="${SUBDIRS} libminisip"
SUBDIRS="${SUBDIRS} minisip"

#If you are debugging, you probably want to build with "g++ -g", to build
#with all debug symbols, useful for coredumps with gdb.
#Comment this line if you don't want to build with debug code
compiler_debug="-ggdb"

#Also useful, you may want to call make with some options ... supply them
#here. 
#For example, -k forces make to keep compiling even there are errors in the 
#   sources. I like this one.
make_options="-k"

#Possible configure commands are shown in: ./configure --help
#   check each folder for more details.

#This are common params, usable in all folders
base_configure_params=""
base_configure_params="$base_configure_params --enable-debug"
base_configure_params="$base_configure_params --disable-static"
#base_configure_params="$base_configure_params --disable-shared"

#set special options for libmutil
libmutil_configure_params=""
#libmutil_configure_params="$libmutil_configure_params --enable-memdebug"

#set special options for libminisip and minisip
#   do a ./configure --help to see ALL available options ... here
#              show just a sample
minisip_configure_params=""
	#the following are forced options for minisip, as we compile for win32
minisip_configure_params="$minisip_configure_params --enable-dsound"
minisip_configure_params="$minisip_configure_params --disable-alsa"
minisip_configure_params="$minisip_configure_params --disable-gconf"
minisip_configure_params="$minisip_configure_params --enable-gtk"
      #--enable-autocall enables automatic calling for debug purposes (default disabled)
      #--enable-ipaq enables various fixes for the iPAQ (default disabled)
      #--enable-ipsec-enable enables ipsec features (default disabled)
      #--enable-aec enables push-2-talk features (default enabled)
      #--enable-video enables video features (default disabled)
      #--enable-textui enables the text based user interface (default disabled)

for subdir in ${SUBDIRS}
do
	echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "Building ${subdir} ... "
	echo "+++++++++++++++++++++++++++++++++++++"
		
	cd ${subdir}

	./bootstrap
	
	configure_params="$base_configure_params"
	
	if [ ${subdir} = "libmutil" ]; then 
		LOC_MUTIL_LIBS=-L$PWD/.libs
		#${subdir}.$LIBS_EXTENSION
		LOC_MUTIL_CFLAGS=-I$PWD/include
		echo libmutil can use special params
		configure_params="$configure_params $libmutil_configure_params"
		MUTIL_LIBS="$LOC_MUTIL_LIBS -lmutil"	\
			MUTIL_CFLAGS=$LOC_MUTIL_CFLAGS \
			CPPFLAGS="-I$prefix_cross_include " 	\
			CXXFLAGS="-Wall $compiler_debug" 	\
			LDFLAGS="-L$prefix_cross_lib"	\
					./configure $host_option $build_option prefix=$prefix_cross $configure_params 
	fi 

	if [ ${subdir} = "libmnetutil" ]; then 
		LOC_MNETUTIL_LIBS=-L$PWD/.libs
		LOC_MNETUTIL_CFLAGS=-I$PWD/include
		MUTIL_LIBS="$LOC_MUTIL_LIBS -lmutil"	\
			MUTIL_CFLAGS=$LOC_MUTIL_CFLAGS \
			CPPFLAGS="-I$prefix_cross_include " 	\
			CXXFLAGS="-Wall $compiler_debug" 	\
			LDFLAGS="-L$prefix_cross_lib"	\
					./configure $host_option $build_option prefix=$prefix_cross $configure_params 
	fi 

	if [ ${subdir} = "libmikey" ]; then 
		LOC_MIKEY_LIBS=-L$PWD/.libs
		LOC_MIKEY_CFLAGS=-I$PWD/include
		MUTIL_LIBS="$LOC_MUTIL_LIBS -lmutil"	\
			MUTIL_CFLAGS=$LOC_MUTIL_CFLAGS \
			CPPFLAGS="-I$prefix_cross_include " 	\
			CXXFLAGS="-Wall $compiler_debug" 	\
			LDFLAGS="-L$prefix_cross_lib"	\
					./configure $host_option $build_option prefix=$prefix_cross $configure_params 
	fi 

	if [ ${subdir} = "libmsip" ]; then 
		LOC_MSIP_LIBS=-L$PWD/.libs
		LOC_MSIP_CFLAGS=-I$PWD/include
		MUTIL_LIBS="$LOC_MUTIL_LIBS -lmutil"	\
			MUTIL_CFLAGS=$LOC_MUTIL_CFLAGS \
			MNETUTIL_LIBS="$LOC_MNETUTIL_LIBS -lmnetutil"	\
			MNETUTIL_CFLAGS=$LOC_MNETUTIL_CFLAGS \
			CPPFLAGS="-I$prefix_cross_include " 	\
			CXXFLAGS="-Wall $compiler_debug" 	\
			LDFLAGS="-L$prefix_cross_lib"	\
					./configure $host_option $build_option prefix=$prefix_cross $configure_params 
	fi 

	if [ ${subdir} = "libminisip" ] ; then 
		echo libminisip can also have special config params
		configure_params="$configure_params $minisip_configure_params"
		MUTIL_LIBS=$LOC_MUTIL_LIBS 	\
			MUTIL_CFLAGS=$LOC_MUTIL_CFLAGS \
			CPPFLAGS="-I$prefix_cross_include " 	\
			CXXFLAGS="-Wall $compiler_debug" 	\
			LDFLAGS="-L$prefix_cross_lib"	\
					./configure $host_option $build_option prefix=$prefix_cross $configure_params 
	fi 
	
	if [ ${subdir} = "minisip" ]; then 
		echo minisip can also have special config params
		configure_params="$configure_params $minisip_configure_params"
		MUTIL_LIBS="$LOC_MUTIL_LIBS -lmutil"	\
			MUTIL_CFLAGS=$LOC_MUTIL_CFLAGS \
			MNETUTIL_LIBS="$LOC_MNETUTIL_LIBS -lmnetutil"	\
			MNETUTIL_CFLAGS=$LOC_MNETUTIL_CFLAGS \
			MIKEY_LIBS="$LOC_MIKEY_LIBS -lmikey"	\
			MIKEY_CFLAGS=$LOC_MIKEY_CFLAGS \
			MSIP_LIBS="$LOC_MSIP_LIBS -lmsip"	\
			MSIP_CFLAGS=$LOC_MSIP_CFLAGS \
			CPPFLAGS="-I$prefix_cross_include " 	\
			CXXFLAGS="-Wall $compiler_debug" 	\
			LDFLAGS="-L$prefix_cross_lib"	\
					./configure $host_option $build_option prefix=$prefix_cross $configure_params 
	fi 
	
	echo "=========================================================="
	echo "configure_params (${subdir})= $configure_params"
	echo "=========================================================="
	
	make $make_options

	cd ..
done

mkdir -p compiled_files
tmp_lib="libmutil"
cp -f $tmp_lib/.libs/$tmp_lib-0.dll compiled_files
cp -f $tmp_lib/.libs/$tmp_lib.dll.a compiled_files
tmp_lib="libmnetutil"
cp -f $tmp_lib/.libs/$tmp_lib-0.dll compiled_files
cp -f $tmp_lib/.libs/$tmp_lib.dll.a compiled_files
tmp_lib="libmikey"
cp -f $tmp_lib/.libs/$tmp_lib-0.dll compiled_files
cp -f $tmp_lib/.libs/$tmp_lib.dll.a compiled_files
tmp_lib="libmsip"
cp -f $tmp_lib/.libs/$tmp_lib-0.dll compiled_files
cp -f $tmp_lib/.libs/$tmp_lib.dll.a compiled_files

cp -f minisip/minisip/minisip.exe compiled_files/minisip_script.exe
cp -f minisip/minisip/.libs/minisip.exe compiled_files/minisip.exe

$strip_bin compiled_files/*

echo "---------------------------------------------------------------------"
echo "Warning:"
echo "   You may need to manually copy the minisip/share/minisip.glade file"
echo "   to the _pkgdatadir_ folder (in debian: /usr/local/share)."
echo " --------------------------------------------------------------------"
echo
