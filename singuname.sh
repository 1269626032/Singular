#!/bin/sh
# 
# FILE:    singuname
# USAGE:   singuname
# PURPOSE: prints to stdout a unique identifier of known uname(s)
# AUTHOR:  obachman 
# CREATED: 5/26/98
#
###########################################################################

egrep="egrep"
uname_a=`uname -a`
devnull='/dev/null'
file=file
binary='/bin/ls'


# HPUX ########################################################
if (echo $uname_a | $egrep "HP-UX" > $devnull) 
then
    prefix=HPUX
    # HPUX-9
    if (echo $uname_a | $egrep "\.09\." > $devnull) 
    then
	echo ${prefix}-9
	exit 0
    # HPUX-10
    elif (echo $uname_a | $egrep "\.10\." > $devnull)
    then
	echo ${prefix}-10
	exit 0
    else
	echo ${prefix}-Unknown
	exit 1
    fi
# Intel ########################################################
elif (echo $uname_a | $egrep "i[3,4,5,6]86" > $devnull)
then
    prefix=ix86
    # Win ################
    if (echo $uname_a | $egrep "CYGWIN" > $devnull)
    then
	echo ${prefix}-Win
	exit 0
    # Linux ###############
    elif (echo $uname_a | $egrep "Linux" > $devnull)
    then
	prefix=${prefix}-Linux
	if (test -x $file && -x $binary)
	then
	    # LinuxAOUT only if  file does not match ELF
	    if ($file $binary | $egrep -v "ELF" > $devnull)
	    then
		echo ${prefix}AOUT
		exit 0
	    fi
	fi
	# everything else is assumed to be Linux ELF    
	echo ${prefix}
	exit 0
    else
	echo ${prefix}-Unknown
	exit 1
    fi
# SunOS ########################################################
elif (echo $uname_a | $egrep "SunOS" > $devnull)
then
    prefix=SunOS
    if (echo $uname_a | $egrep "4\.[0-9]" > $devnull) 
    then
	echo ${prefix}-4
	exit 0
    # HPUX-10
    elif (echo $uname_a | $egrep "5\.[0-9]" > $devnull)
    then
	echo ${prefix}-5
	exit 0
    else
	echo ${prefix}-Unknown
	exit 1
    fi
# DecAlpha ########################################################
elif (echo $uname_a | $egrep "Alpha" > $devnull)
then
    prefix=DecAlpha
    if (echo $uname_a | $egrep "Linux" > $devnull)
    then
	echo ${prefix}-Linux
	exit 0 
    else
	echo ${prefix}-Unknown
	exit 1 
    fi
# IRIX ########################################################
elif (echo $uname_a | $egrep "IRIX" > $devnull)
then
    prefix=IRIX
    if (echo $uname_a | $egrep "6\.[0-9]" > $devnull)
    then
	echo ${prefix}-6
	exit 0 
    else
	echo ${prefix}-Unknown
	exit 1 
    fi
# AIX ########################################################
elif (echo $uname_a | $egrep "AIX" > $devnull)
then
    prefix=AIX
    if (uname -v |  $egrep "4" > $devnull)
    then
	echo ${prefix}-4
	exit 0
    elif (uname -v |  $egrep "3" > $devnull)
    then
	echo ${prefix}-3
	exit 0
    else
	echo ${prefix}-Unknown
	exit 1
    fi
else # Unknown ########################################################
    echo Unknown
    exit 2
fi


