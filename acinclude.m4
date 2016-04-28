# Configure paths for ESD
# Manish Singh    98-9-30
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_ESD([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for ESD, and define ESD_CFLAGS and ESD_LIBS
dnl
AC_DEFUN([AM_PATH_ESD],
[dnl 
dnl Get the cflags and libraries from the esd-config script
dnl
AC_ARG_WITH(esd-prefix,[  --with-esd-prefix=PFX   Prefix where ESD is installed (optional)],
            esd_prefix="$withval", esd_prefix="")
AC_ARG_WITH(esd-exec-prefix,[  --with-esd-exec-prefix=PFX Exec prefix where ESD is installed (optional)],
            esd_exec_prefix="$withval", esd_exec_prefix="")
AC_ARG_ENABLE(esdtest, [  --disable-esdtest       Do not try to compile and run a test ESD program],
		    , enable_esdtest=yes)

  if test x$esd_exec_prefix != x ; then
     esd_args="$esd_args --exec-prefix=$esd_exec_prefix"
     if test x${ESD_CONFIG+set} != xset ; then
        ESD_CONFIG=$esd_exec_prefix/bin/esd-config
     fi
  fi
  if test x$esd_prefix != x ; then
     esd_args="$esd_args --prefix=$esd_prefix"
     if test x${ESD_CONFIG+set} != xset ; then
        ESD_CONFIG=$esd_prefix/bin/esd-config
     fi
  fi

  AC_PATH_PROG(ESD_CONFIG, esd-config, no)
  min_esd_version=ifelse([$1], ,0.2.7,$1)
  AC_MSG_CHECKING(for ESD - version >= $min_esd_version)
  no_esd=""
  if test "$ESD_CONFIG" = "no" ; then
    no_esd=yes
  else
    AC_LANG_SAVE
    AC_LANG_C
    ESD_CFLAGS=`$ESD_CONFIG $esdconf_args --cflags`
    ESD_LIBS=`$ESD_CONFIG $esdconf_args --libs`

    esd_major_version=`$ESD_CONFIG $esd_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    esd_minor_version=`$ESD_CONFIG $esd_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    esd_micro_version=`$ESD_CONFIG $esd_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_esdtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $ESD_CFLAGS"
      LIBS="$LIBS $ESD_LIBS"
dnl
dnl Now check if the installed ESD is sufficiently new. (Also sanity
dnl checks the results of esd-config to some extent
dnl
      rm -f conf.esdtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esd.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.esdtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_esd_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_esd_version");
     exit(1);
   }

   if (($esd_major_version > major) ||
      (($esd_major_version == major) && ($esd_minor_version > minor)) ||
      (($esd_major_version == major) && ($esd_minor_version == minor) && ($esd_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'esd-config --version' returned %d.%d.%d, but the minimum version\n", $esd_major_version, $esd_minor_version, $esd_micro_version);
      printf("*** of ESD required is %d.%d.%d. If esd-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If esd-config was wrong, set the environment variable ESD_CONFIG\n");
      printf("*** to point to the correct copy of esd-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_esd=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
       AC_LANG_RESTORE
     fi
  fi
  if test "x$no_esd" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$ESD_CONFIG" = "no" ; then
       echo "*** The esd-config script installed by ESD could not be found"
       echo "*** If ESD was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the ESD_CONFIG environment variable to the"
       echo "*** full path to esd-config."
     else
       if test -f conf.esdtest ; then
        :
       else
          echo "*** Could not run ESD test program, checking why..."
          CFLAGS="$CFLAGS $ESD_CFLAGS"
          LIBS="$LIBS $ESD_LIBS"
          AC_LANG_SAVE
          AC_LANG_C
          AC_TRY_LINK([
#include <stdio.h>
#include <esd.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding ESD or finding the wrong"
          echo "*** version of ESD. If it is not finding ESD, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means ESD was incorrectly installed"
          echo "*** or that you have moved ESD since it was installed. In the latter case, you"
          echo "*** may want to edit the esd-config script: $ESD_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
          AC_LANG_RESTORE
       fi
     fi
     ESD_CFLAGS=""
     ESD_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(ESD_CFLAGS)
  AC_SUBST(ESD_LIBS)
  rm -f conf.esdtest
])

dnl AM_ESD_SUPPORTS_MULTIPLE_RECORD([ACTION-IF-SUPPORTS [, ACTION-IF-NOT-SUPPORTS]])
dnl Test, whether esd supports multiple recording clients (version >=0.2.21)
dnl
AC_DEFUN([AM_ESD_SUPPORTS_MULTIPLE_RECORD],
[dnl
  AC_MSG_NOTICE([whether installed esd version supports multiple recording clients])
  ac_save_ESD_CFLAGS="$ESD_CFLAGS"
  ac_save_ESD_LIBS="$ESD_LIBS"
  AM_PATH_ESD(0.2.21,
    ifelse([$1], , [
      AM_CONDITIONAL(ESD_SUPPORTS_MULTIPLE_RECORD, true)
      AC_DEFINE(ESD_SUPPORTS_MULTIPLE_RECORD, 1,
	[Define if you have esound with support of multiple recording clients.])],
    [$1]),
    ifelse([$2], , [AM_CONDITIONAL(ESD_SUPPORTS_MULTIPLE_RECORD, false)], [$2])
    if test "x$ac_save_ESD_CFLAGS" != x ; then
       ESD_CFLAGS="$ac_save_ESD_CFLAGS"
    fi
    if test "x$ac_save_ESD_LIBS" != x ; then
       ESD_LIBS="$ac_save_ESD_LIBS"
    fi
  )
])

dnl Check if we have SDL (sdl-config, header and library) version >= 1.2.0
dnl Extra options: --with-sdl-prefix=PATH and --with-sdl={sdl12,sdl2}
dnl Output:
dnl SDL_CFLAGS and SDL_LIBS are set and AC_SUBST-ed
dnl HAVE_SDL_H is AC_DEFINE-d
AC_DEFUN([EXULT_CHECK_SDL],[
  exult_backupcppflags="$CPPFLAGS"
  exult_backupldflags="$LDFLAGS"
  exult_backuplibs="$LIBS"

  exult_sdlok=yes

  AC_ARG_WITH(sdl-prefix,[  --with-sdl-prefix=PFX   Prefix where SDL is installed (optional)], sdl_prefix="$withval", sdl_prefix="")
  AC_ARG_WITH(sdl-exec-prefix,[  --with-sdl-exec-prefix=PFX Exec prefix where SDL is installed (optional)], sdl_exec_prefix="$withval", sdl_exec_prefix="")
  AC_ARG_WITH(sdl,       [  --with-sdl=sdl12,sdl2   Select a specific version of SDL to use (optional)], sdl_ver="$withval", sdl_ver="")

  dnl First: find sdl-config or sdl2-config
  exult_extra_path=$prefix/bin:$prefix/usr/bin
  sdl_args=""
  if test x$sdl_exec_prefix != x ; then
     sdl_args="$sdl_args --exec-prefix=$sdl_exec_prefix"
     exult_extra_path=$sdl_exec_prefix/bin
  fi
  if test x$sdl_prefix != x ; then
     sdl_args="$sdl_args --prefix=$sdl_prefix"
     exult_extra_path=$sdl_prefix/bin
  fi
  if test x"$sdl_ver" = xsdl12 ; then
    exult_sdl_progs=sdl-config
  elif test x"$sdl_ver" = xsdl2 ; then
    exult_sdl_progs=sdl2-config
  else
    dnl NB: This line implies we prefer SDL 1.2 to SDL 2.0
    exult_sdl_progs="sdl-config sdl2-config"
  fi
  AC_PATH_PROGS(SDL_CONFIG, $exult_sdl_progs, no, [$exult_extra_path:$PATH])
  if test "$SDL_CONFIG" = "no" ; then
    exult_sdlok=no
  else
    SDL_CFLAGS=`$SDL_CONFIG $sdl_args --cflags`
    SDL_LIBS=`$SDL_CONFIG $sdl_args --libs`

    sdl_major_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdl_minor_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdl_patchlevel=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test $sdl_major_version -eq 1 ; then
      sdl_ver=sdl12
    else
      sdl_ver=sdl2
    fi
  fi

  if test x"$sdl_ver" = xsdl2 ; then
    REQ_MAJOR=2
    REQ_MINOR=0
    REQ_PATCHLEVEL=0
  else
    REQ_MAJOR=1
    REQ_MINOR=2
    REQ_PATCHLEVEL=0
  fi
  REQ_VERSION=$REQ_MAJOR.$REQ_MINOR.$REQ_PATCHLEVEL

  AC_MSG_CHECKING([for SDL - version >= $REQ_VERSION])


  dnl Second: include "SDL.h"

  if test x$exult_sdlok = xyes ; then
    CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
    #include "SDL.h"

    int main(int argc, char *argv[])
    {
      return 0;
    }
    ]],)],sdlh_found=yes,sdlh_found=no)

    if test x$sdlh_found = xno; then
      exult_sdlok=no
    else
      AC_DEFINE(HAVE_SDL_H, 1, [Define to 1 if you have the "SDL.h" header file])
    fi
  fi

  dnl Next: version check (cross-compile-friendly idea borrowed from autoconf)
  dnl (First check version reported by sdl-config, then confirm
  dnl  the version in SDL.h matches it)

  if test x$exult_sdlok = xyes ; then

    if test ! \( \( $sdl_major_version -gt $REQ_MAJOR \) -o \( \( $sdl_major_version -eq $REQ_MAJOR \) -a \( \( $sdl_minor_version -gt $REQ_MINOR \) -o \( \( $sdl_minor_version -eq $REQ_MINOR \) -a \( $sdl_patchlevel -gt $REQ_PATCHLEVEL \) \) \) \) \); then
      exult_sdlok="no, version < $REQ_VERSION found"
    else
      AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
      #include "SDL.h"

      int main(int argc, char *argv[])
      {
        static int test_array[1-2*!(SDL_MAJOR_VERSION==$sdl_major_version&&SDL_MINOR_VERSION==$sdl_minor_version&&SDL_PATCHLEVEL==$sdl_patchlevel)];
        test_array[0] = 0;
        return 0;
      }
      ]])],,[[exult_sdlok="no, version of SDL.h doesn't match that of sdl-config"]])
    fi
  fi

  dnl Next: try linking

  if test "x$exult_sdlok" = xyes; then
    LIBS="$LIBS $SDL_LIBS"

    AC_LINK_IFELSE([AC_LANG_SOURCE([[
    #include "SDL.h"

    int main(int argc, char* argv[]) {
      SDL_Init(0);
      return 0;
    }
    ]])],sdllinkok=yes,sdllinkok=no)
    if test x$sdllinkok = xno; then
      exult_sdlok=no
    fi
  fi

  AC_MSG_RESULT($exult_sdlok)

  LDFLAGS="$exult_backupldflags"
  CPPFLAGS="$exult_backupcppflags"
  LIBS="$exult_backuplibs"

  if test "x$exult_sdlok" = xyes; then
    AC_SUBST(SDL_CFLAGS)
    AC_SUBST(SDL_LIBS)
    ifelse([$1], , :, [$1])
  else
    ifelse([$2], , :, [$2])
  fi
]);

dnl Configure Paths for Alsa
dnl Some modifications by Richard Boulton <richard-alsa@tartarus.org>
dnl Christopher Lansdown <lansdoct@cs.alfred.edu>
dnl Jaroslav Kysela <perex@suse.cz>
dnl Last modification: alsa.m4,v 1.24 2004/09/15 18:48:07 tiwai Exp
dnl AM_PATH_ALSA([MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for libasound, and define ALSA_CFLAGS and ALSA_LIBS as appropriate.
dnl enables arguments --with-alsa-prefix=
dnl                   --with-alsa-enc-prefix=
dnl                   --disable-alsatest
dnl
dnl For backwards compatibility, if ACTION_IF_NOT_FOUND is not specified,
dnl and the alsa libraries are not found, a fatal AC_MSG_ERROR() will result.
dnl
AC_DEFUN([AM_PATH_ALSA],
[dnl Save the original CFLAGS, LDFLAGS, and LIBS
alsa_save_CFLAGS="$CFLAGS"
alsa_save_LDFLAGS="$LDFLAGS"
alsa_save_LIBS="$LIBS"
alsa_found=yes

dnl
dnl Get the cflags and libraries for alsa
dnl
AC_ARG_WITH(alsa-prefix,
[  --with-alsa-prefix=PFX  Prefix where Alsa library is installed(optional)],
[alsa_prefix="$withval"], [alsa_prefix=""])

AC_ARG_WITH(alsa-inc-prefix,
[  --with-alsa-inc-prefix=PFX  Prefix where include libraries are (optional)],
[alsa_inc_prefix="$withval"], [alsa_inc_prefix=""])

dnl FIXME: this is not yet implemented
AC_ARG_ENABLE(alsatest,
[  --disable-alsatest      Do not try to compile and run a test Alsa program],
[enable_alsatest="$enableval"],
[enable_alsatest=yes])

dnl Add any special include directories
AC_MSG_CHECKING(for ALSA CFLAGS)
if test "$alsa_inc_prefix" != "" ; then
	ALSA_CFLAGS="$ALSA_CFLAGS -I$alsa_inc_prefix"
	CFLAGS="$CFLAGS -I$alsa_inc_prefix"
fi
AC_MSG_RESULT($ALSA_CFLAGS)
CFLAGS="$alsa_save_CFLAGS"

dnl add any special lib dirs
AC_MSG_CHECKING(for ALSA LDFLAGS)
if test "$alsa_prefix" != "" ; then
	ALSA_LIBS="$ALSA_LIBS -L$alsa_prefix"
	LDFLAGS="$LDFLAGS $ALSA_LIBS"
fi

dnl add the alsa library
ALSA_LIBS="$ALSA_LIBS -lasound -lm -ldl -lpthread"
LIBS="$ALSA_LIBS $LIBS"
AC_MSG_RESULT($ALSA_LIBS)

dnl Check for a working version of libasound that is of the right version.
min_alsa_version=ifelse([$1], ,0.1.1,$1)
AC_MSG_CHECKING(for libasound headers version >= $min_alsa_version)
no_alsa=""
    alsa_min_major_version=`echo $min_alsa_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    alsa_min_minor_version=`echo $min_alsa_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    alsa_min_micro_version=`echo $min_alsa_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

AC_LANG_SAVE
AC_LANG_C
AC_TRY_COMPILE([
#include <alsa/asoundlib.h>
], [
/* ensure backward compatibility */
#if !defined(SND_LIB_MAJOR) && defined(SOUNDLIB_VERSION_MAJOR)
#define SND_LIB_MAJOR SOUNDLIB_VERSION_MAJOR
#endif
#if !defined(SND_LIB_MINOR) && defined(SOUNDLIB_VERSION_MINOR)
#define SND_LIB_MINOR SOUNDLIB_VERSION_MINOR
#endif
#if !defined(SND_LIB_SUBMINOR) && defined(SOUNDLIB_VERSION_SUBMINOR)
#define SND_LIB_SUBMINOR SOUNDLIB_VERSION_SUBMINOR
#endif

#  if(SND_LIB_MAJOR > $alsa_min_major_version)
  exit(0);
#  else
#    if(SND_LIB_MAJOR < $alsa_min_major_version)
#       error not present
#    endif

#   if(SND_LIB_MINOR > $alsa_min_minor_version)
  exit(0);
#   else
#     if(SND_LIB_MINOR < $alsa_min_minor_version)
#          error not present
#      endif

#      if(SND_LIB_SUBMINOR < $alsa_min_micro_version)
#        error not present
#      endif
#    endif
#  endif
exit(0);
],
  [AC_MSG_RESULT(found.)],
  [AC_MSG_RESULT(not present.)
   ifelse([$3], , [AC_MSG_ERROR(Sufficiently new version of libasound not found.)])
   alsa_found=no]
)
AC_LANG_RESTORE

dnl Now that we know that we have the right version, let's see if we have the library and not just the headers.
if test "x$enable_alsatest" = "xyes"; then
AC_CHECK_LIB([asound], [snd_ctl_open],,
	[ifelse([$3], , [AC_MSG_ERROR(No linkable libasound was found.)])
	 alsa_found=no]
)
fi

LDFLAGS="$alsa_save_LDFLAGS"
LIBS="$alsa_save_LIBS"

if test "x$alsa_found" = "xyes" ; then
   ifelse([$2], , :, [$2])
else
   ALSA_CFLAGS=""
   ALSA_LIBS=""
   ifelse([$3], , :, [$3])
fi

dnl That should be it.  Now just export out symbols:
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)
])

# ao.m4
# Configure paths for libao
# Jack Moffitt <jack@icecast.org> 10-21-2000
# Shamelessly stolen from Owen Taylor and Manish Singh

dnl XIPH_PATH_AO([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for libao, and define AO_CFLAGS and AO_LIBS
dnl
AC_DEFUN([XIPH_PATH_AO],
[dnl 
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(ao,[  --with-ao=PFX   Prefix where libao is installed (optional)], ao_prefix="$withval", ao_prefix="")
AC_ARG_WITH(ao-libraries,[  --with-ao-libraries=DIR   Directory where libao library is installed (optional)], ao_libraries="$withval", ao_libraries="")
AC_ARG_WITH(ao-includes,[  --with-ao-includes=DIR   Directory where libao header files are installed (optional)], ao_includes="$withval", ao_includes="")
AC_ARG_ENABLE(aotest, [  --disable-aotest       Do not try to compile and run a test ao program],, enable_aotest=yes)


  if test "x$ao_libraries" != "x" ; then
    AO_LIBS="-L$ao_libraries"
  elif test "x$ao_prefix" != "x"; then
    AO_LIBS="-L$ao_prefix/lib"
  elif test "x$prefix" != "xNONE"; then
    AO_LIBS="-L$prefix/lib"
  fi

  if test "x$ao_includes" != "x" ; then
    AO_CFLAGS="-I$ao_includes"
  elif test "x$ao_prefix" != "x"; then
    AO_CFLAGS="-I$ao_prefix/include"
  elif test "x$prefix" != "xNONE"; then
    AO_CFLAGS="-I$prefix/include"
  fi

  # see where dl* and friends live
  AC_CHECK_FUNCS(dlopen, [AO_DL_LIBS=""], [
    AC_CHECK_LIB(dl, dlopen, [AO_DL_LIBS="-ldl"], [
      AC_MSG_WARN([could not find dlopen() needed by libao sound drivers
      your system may not be supported.])
    ])
  ])

  AO_LIBS="$AO_LIBS -lao $AO_DL_LIBS"

  AC_MSG_CHECKING(for ao)
  no_ao=""


  if test "x$enable_aotest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $AO_CFLAGS"
    LIBS="$LIBS $AO_LIBS"
dnl
dnl Now check if the installed ao is sufficiently new.
dnl
      rm -f conf.aotest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ao/ao.h>

int main ()
{
  system("touch conf.aotest");
  return 0;
}

],, no_ao=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
  fi

  if test "x$no_ao" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])     
  else
     AC_MSG_RESULT(no)
     if test -f conf.aotest ; then
       :
     else
       echo "*** Could not run ao test program, checking why..."
       CFLAGS="$CFLAGS $AO_CFLAGS"
       LIBS="$LIBS $AO_LIBS"
       AC_TRY_LINK([
#include <stdio.h>
#include <ao/ao.h>
],     [ return 0; ],
       [ echo "*** The test program compiled, but did not run. This usually means"
       echo "*** that the run-time linker is not finding ao or finding the wrong"
       echo "*** version of ao. If it is not finding ao, you'll need to set your"
       echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
       echo "*** to the installed location  Also, make sure you have run ldconfig if that"
       echo "*** is required on your system"
       echo "***"
       echo "*** If you have an old version installed, it is best to remove it, although"
       echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
       [ echo "*** The test program failed to compile or link. See the file config.log for the"
       echo "*** exact error that occured. This usually means ao was incorrectly installed"
       echo "*** or that you have moved ao since it was installed." ])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
     AO_CFLAGS=""
     AO_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(AO_CFLAGS)
  AC_SUBST(AO_LIBS)
  rm -f conf.aotest
])
