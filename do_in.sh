#!/bin/sh
# update the two variables below as necessary

OTAWA_POLY=/home/forget/Code/polymalys/otawa
OTAWA_WSYMB=/home/forget/Installs/otawa-wsymb/otawa

case $1 in
    "polymalys" )
	export PATH=$OTAWA_POLY/bin/:$PATH
	export LD_LIBRARY_PATH=$OTAWA_POLY/lib/:$OTAWA_POLY/lib/otawa/otawa/:$LD_LIBRARY_PATH
	shift
	$*;;
    "wsymb" )
	export PATH=$OTAWA_WSYMB/bin:$PATH
	export LD_LIBRARY_PATH=$OTAWA_WSYMB/lib:$OTAWA_WSYMB/lib/otawa/otawa:$LD_LIBRARY_PATH
	shift
	$*;;
    * ) echo "Unrecognized option"
esac
