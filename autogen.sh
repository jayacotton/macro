#!/bin/sh
# *****************************************************************************
# Filename:			autogen.sh
# Description:
# Copyright(c):
# Author(s):		Jay Cotton
# Created:		18 Feb 2015	
# Last modified:	18 Feb 2015	
# *****************************************************************************


echo "Generating configure files... may take a while."


aclocal
libtoolize --automake
automake -a
autoconf


