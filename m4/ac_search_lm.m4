# check_search_lm.m4 - Macro to locate textcat.lm files. -*- Autoconf -*-
# serial 1
#
# Copyright © 2018 Ko van der Sloot <K.vanderSloot@let.ru.nl>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# As a special exception to the GNU General Public License, if you
# distribute this file as part of a program that contains a
# configuration script generated by Autoconf, you may include it under
# the same distribution terms that you use for the rest of that program.

# AC_SEARCH_LM()
# ----------------------------------
AC_DEFUN([AC_SEARCH_LM],
  [tcdirs="/usr/share/libtextcat /usr/share/libexttextcat /usr/local/share/libtextcat /usr/local/share/libexttextcat /usr/local/Cellar/libtextcat/2.2/share/LM "

   for d in $tcdirs
   do
     if test -f ${d}/nl.lm
     then
       MODULE_PREFIX=$d
       AC_SUBST([MODULE_PREFIX])
       AM_CONDITIONAL([OLD_LM], [test 1 = 0])
       break
     fi
   done

   if test "x$MODULE_PREFIX" = "x"
   then
     for d in $tcdirs
     do
       if test -f ${d}/dutch.lm
       then
         MODULE_PREFIX=$d
         AC_SUBST([MODULE_PREFIX])
	 AM_CONDITIONAL([OLD_LM], [test 1 = 1])
         break
       fi
     done
   fi
   if test "x$MODULE_PREFIX" = "x"
   then
     AC_MSG_NOTICE([textcat Language Model files not found. Textcat disabled.])
   else
     TEXTCAT_FOUND=1
   fi

 ])