#!/bin/sh
# update the two first variables as necessary

OTAWA_POLY=/home/forget/Code/WSymb-procedures/polymalys/otawa
OTAWA_WSYMB=/home/forget/Code/WSymb-procedures/otawa

case $1 in
    "polymalys" )
	echo "Executing with Otawa version Polymalys"
	export PATH=$OTAWA_POLY/bin/:$PATH
	export LD_LIBRARY_PATH=$OTAWA_POLY/lib/:$OTAWA_POLY/lib/otawa/otawa/:$LD_LIBRARY_PATH
	shift
	$*;;
    "wsymb" )
	echo "Executing Otawa version WSymb"
	export PATH=$OTAWA_WSYMB/bin:$PATH
	export LD_LIBRARY_PATH=$OTAWA_WSYMB/lib:$OTAWA_WSYMB/lib/otawa/otawa:$LD_LIBRARY_PATH
	shift
	$*;;
    * ) echo "Unrecognized option"
esac
