%define name adplay
%define version 1.2
%define release 1mdk

Summary: AdLib music player for the command line
Name: %{name}
Version: %{version}
Release: %{release}
Source0: %{name}-%{version}.tar.bz2
License: GPL
Group: Sound
URL: http://adplug.sourceforge.net
BuildRoot: %{_tmppath}/%{name}-buildroot
Prefix: %{_prefix}
BuildRequires: libadplug-devel >= 1.1

%description
AdPlay/UNIX is AdPlug's UNIX console-based frontend. AdPlug is a free,
universal OPL2 audio playback library. AdPlay/UNIX supports the full range
of AdPlug's file format playback features. Despite this, at the moment, only
emulated OPL2 output is supported by AdPlay/UNIX, but this on a wide range
of output devices.

%prep
%setup -q

%build
%configure2_5x
%make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall_std

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README NEWS TODO AUTHORS ChangeLog
%_bindir/adplay
%_mandir/man1/adplay.1*

%changelog
* Tue Nov 26 2002 Götz Waschk <waschk@linux-mandrake.com> 1.2-1mdk
- initial package

