# Tell autoconf we're compiling a C++ program using automake and libtool.
AC_INIT(adplay,1.8)
AC_CONFIG_SRCDIR(src/adplay.cc)
AC_CONFIG_FILES(Makefile src/Makefile doc/Makefile)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(src/config.h)
AM_PROG_LIBTOOL
AC_LANG(C++)

# Check if we got a sane C/C++ compilation environment.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CXXCPP

# Nothing works without these libraries...
AC_CHECK_LIB(stdc++,main,,AC_MSG_ERROR([libstdc++ not installed]))
PKG_CHECK_MODULES([adplug], [adplug >= 2.0],,[
AC_MSG_WARN([You seem to be using a version of AdPlug prior to 2.0. \
I will try to do the old-style library search for which i cannot check \
versions. Please bear in mind that i am requiring at least version 1.4.])
AC_CHECK_LIB(adplug,main,,AC_MSG_ERROR([*** AdPlug not installed ***]))])

# Check if getopt header is installed on this system
AC_CHECK_HEADERS([getopt.h], ,
	AC_SUBST(GETOPT_SOURCES, [getopt.c getopt1.c getopt.h]))

# Save compiler flags and set up for compiling test programs
oldlibs="$LIBS"
oldcppflags="$CPPFLAGS"
LIBS="$LDFLAGS $adplug_LIBS"
CPPFLAGS="$CPPFLAGS $adplug_CFLAGS"

# Check if AdPlug is installed and linked correctly
AC_MSG_CHECKING([whether AdPlug is linked correctly])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <adplug/player.h>

class Testplayer: public CPlayer
{
  public:
    Testplayer(): CPlayer(NULL) {}
    bool load(const std::string &f, const CFileProvider &fp) { return false; }
    bool update() { return false; }
    float getrefresh() { return 0; }
    std::string gettype() { return std::string(); }
    void rewind(int s) {}
};

Testplayer p;], [p.getrefresh();])],
  AC_MSG_RESULT([yes]), AC_MSG_RESULT([no]); AC_MSG_ERROR([Unable to compile a program using AdPlug. Please check to ensure AdPlug is installed correctly.]) )

# Check if AdPlug supports the new getsubsong() method
AC_MSG_CHECKING([whether AdPlug supports the getsubsong() method])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <adplug/player.h>

class Testplayer: public CPlayer
{
  public:
    Testplayer(): CPlayer(NULL) {}
    bool load(const std::string &f, const CFileProvider &fp) { return false; }
    bool update() { return false; }
    float getrefresh() { return 0; }
    std::string gettype() { return std::string(); }
    void rewind(int s) {}
};

Testplayer p;], [p.getsubsong();])],
   AC_DEFINE(HAVE_ADPLUG_GETSUBSONG,, [Defined if AdPlug supports the getsubsong() method])
AC_MSG_RESULT([yes]), AC_MSG_RESULT([no]))

# Check if AdPlug supports the new surround/harmonic synth
AC_MSG_CHECKING([whether AdPlug supports the surround/harmonic synth])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <adplug/surroundopl.h>], [CSurroundopl *dummy;])],
   AC_DEFINE(HAVE_ADPLUG_SURROUND,, [Defined if AdPlug supports the surround/harmonic synth])
AC_MSG_RESULT([yes]), AC_MSG_RESULT([no - AdPlug >= 2.2 required]))

# Check if AdPlug supports NukedOPL
AC_MSG_CHECKING([whether AdPlug supports the NukedOPL synth])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <adplug/nemuopl.h>], [CNemuopl *dummy;])],
   AC_DEFINE(HAVE_ADPLUG_NUKEDOPL,, [Defined if AdPlug supports the NukedOPL synth])
AC_MSG_RESULT([yes]), AC_MSG_RESULT([no - AdPlug >= 2.3 required]))

# Restore flags after compiling test programs
LIBS="$oldlibs"
CPPFLAGS="$oldcppflags"

##### Output mechanism checks #####
# These checks enable or disable certain output mechanisms,
# depending on system facilities and user requests.

# Define user option arguments
AC_ARG_ENABLE([output-oss],AC_HELP_STRING([--disable-output-oss],[Disable OSS output]))
AC_ARG_ENABLE([output-null],AC_HELP_STRING([--disable-output-null],[Disable null output]))
AC_ARG_ENABLE([output-disk],AC_HELP_STRING([--disable-output-disk],[Disable disk writer]))
AC_ARG_ENABLE([output-esound],AC_HELP_STRING([--disable-output-esound],[Disable EsounD output]))
AC_ARG_ENABLE([output-qsa],AC_HELP_STRING([--disable-output-qsa],[Disable QSA output]))
AC_ARG_ENABLE([output-sdl],AC_HELP_STRING([--disable-output-sdl],[Disable SDL output]))
AC_ARG_ENABLE([output-alsa],AC_HELP_STRING([--disable-output-alsa],[Disable ALSA output]))
AC_ARG_ENABLE([output-ao],AC_HELP_STRING([--disable-output-ao],[Disable AO output]))
# Check if we can compile the enabled drivers:
# OSS driver
if test ${enable_output_oss:=yes} = yes; then
   AC_MSG_CHECKING([for OSS headers])
   AC_TRY_CPP([
                #include <fcntl.h>
                #include <sys/ioctl.h>
       		#include <sys/soundcard.h>
              ],[
               	AC_DEFINE(DRIVER_OSS,1,[Build OSS driver])
               	drivers=$drivers' oss.$(OBJEXT)'
               	AC_MSG_RESULT([found])
              ],[
               	enable_output_oss=no
               	AC_MSG_RESULT([not found -- OSS output disabled])
              ])
fi

# AO output
if test ${enable_output_ao:=yes} = yes; then
   XIPH_PATH_AO(AC_DEFINE(DRIVER_AO,1,[Build AO output])
   drivers=$drivers' ao.${OBJEXT}',
   enable_output_ao=no
   AC_MSG_RESULT([*** AO (libao) not installed ***]))
fi

# Null output
if test ${enable_output_null:=yes} = yes; then
   AC_DEFINE(DRIVER_NULL,1,[Build null output])
fi

# Disk writer
if test ${enable_output_disk:=yes} = yes; then
   AC_DEFINE(DRIVER_DISK,1,[Build disk writer])
   drivers=$drivers' disk.$(OBJEXT)'
fi

# EsounD output
if test ${enable_output_esound:=yes} = yes; then
   AM_PATH_ESD(0.2.8,
      AC_DEFINE(DRIVER_ESOUND,1,[Build EsounD output])
      drivers=$drivers' esound.$(OBJEXT)',
      enable_output_esound=no
      AC_MSG_RESULT([*** EsounD (libesd) >= 0.2.8 not installed ***]))
fi

# QSA driver
if test ${enable_output_qsa:=yes} = yes; then
   AC_MSG_CHECKING([for QSA headers])
   AC_TRY_COMPILE([
                  #include <sys/asoundlib.h>
                  #include <sys/neutrino.h>
                  ],[
                  int arg=snd_pcm_open_preferred(NULL, NULL, NULL, 0);
                  ],[
                  have_qsa=yes
                  ])
    if test x$have_qsa = xyes; then
	AC_DEFINE(DRIVER_QSA,1,[Build QSA driver])
	drivers=$drivers' qsa.$(OBJEXT)'
        QSA_LIBS=-lasound
        AC_SUBST(QSA_LIBS)
	AC_MSG_RESULT([found])
    else
	enable_output_qsa=no
        QSA_LIBS=""
        AC_SUBST(QSA_LIBS)
	AC_MSG_RESULT([not found -- QSA output disabled])
    fi
fi

# SDL output
if test ${enable_output_sdl:=yes} = yes; then
   AC_LANG_PUSH(C)
   EXULT_CHECK_SDL(
	AC_DEFINE(DRIVER_SDL,1,[Build SDL output])
	drivers=$drivers' sdl.$(OBJEXT)',
	enable_output_sdl=no
	AC_MSG_RESULT([*** SDL (libsdl) >= 1.2.0 not installed ***]))
   AC_LANG_POP(C)
fi

# ALSA output
if test ${enable_output_alsa:=yes} = yes; then
   AM_PATH_ALSA(0.9.1,
	AC_DEFINE(DRIVER_ALSA,1,[Build ALSA output])
	drivers=$drivers' alsa.${OBJEXT}',
	enable_output_alsa=no
	AC_MSG_RESULT([*** ALSA (libasound) >= 0.9.1 not installed ***]))
fi



AC_SUBST([drivers])

AC_OUTPUT

# Display user informative configuration output
echo ""
echo "Configuration:"
echo "Install path:             ${prefix}"
echo ""
echo "Build output mechanisms:"
echo "OSS output (oss):         ${enable_output_oss}"
echo "Null output (null):       ${enable_output_null}"
echo "Disk writer (disk):       ${enable_output_disk}"
echo "EsounD output (esound):   ${enable_output_esound}"
echo "QSA output (qsa):         ${enable_output_qsa}"
echo "SDL output (sdl):         ${enable_output_sdl}"
echo "ALSA output (alsa):       ${enable_output_alsa}"
echo "Libao output (ao):        ${enable_output_ao}"

if test "x$exult_sdlok" = xyes; then
   echo ""
   echo "SDL version:              `$SDL_CONFIG $sdl_args --version`"
fi
