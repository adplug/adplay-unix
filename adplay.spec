%define name adplay
%define version 1.7
%define release 1

Summary: AdLib music player for the command line
Name: %{name}
Version: %{version}
Release: %{release}
Source0: %{name}-%{version}.tar.bz2
License: GPL
Group: Applications/Multimedia
URL: http://adplug.sourceforge.net
BuildRoot: %{_tmppath}/%{name}-buildroot
Prefix: %{_prefix}
BuildRequires: adplug-devel >= 1.4
Requires: adplug >= 1.4

%description
AdPlay/UNIX is AdPlug's UNIX console-based frontend. AdPlug is a free,
universal OPL2 audio playback library. AdPlay/UNIX supports the full range
of AdPlug's file format playback features. Despite this, at the moment, only
emulated OPL2 output is supported by AdPlay/UNIX, but this on a wide range
of output devices.

%prep
%setup -q

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README NEWS TODO AUTHORS ChangeLog
%_bindir/adplay
%_mandir/man1/adplay.1*

%changelog
* Tue Mar  4 2003 Götz Waschk <waschk@linux-mandrake.com> 1.3-1
- requires new adplug libraries
- fix group for RH standard
- new version

* Tue Nov 26 2002 Götz Waschk <waschk@linux-mandrake.com> 1.2-1
- initial package

