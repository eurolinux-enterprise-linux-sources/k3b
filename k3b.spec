
Name:    k3b
Summary: CD/DVD/Blu-ray burning application
Epoch:   1
Version: 2.0.2
Release: 17%{?dist}

Group:   Applications/Archiving
License: GPLv2+
URL:     http://www.k3b.org/
Source0: http://downloads.sourceforge.net/sourceforge/k3b/k3b-%{version}%{?pre}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# ie, where cdrecord and friends aren't present
ExcludeArch: s390 s390x

# Remove WebKit on RHEL
Patch1: k3b-2.0.2-no-webkit.patch

## upstreamable patches
Patch50: k3b-2.0.2-use_vartmp_instead_of_tmp.patch

## upstream patches
# wodim(cdrecord) doesnt work for DVDs/BluRay, use growisofs instead
# http://bugzilla.redhat.com/610976
# https://git.reviewboard.kde.org/r/101208/
# https://projects.kde.org/projects/extragear/multimedia/k3b/repository/revisions/1853eee0f15d9d5a1ab0407d5d87e36167e5c9eb
Patch100: k3b-2.0.2-use_growisofs_instead_of_wodim.patch
# increase overburning tolerance from 1/10 to 1/4 for CD-R90/99 media
# http://bugs.kde.org/show_bug.cgi?id=276002
# https://git.reviewboard.kde.org/r/102648/
# https://projects.kde.org/projects/extragear/multimedia/k3b/repository/revisions/b0df9e7771ac9148004187bb53ba13ac40b01cae
Patch101: k3b-2.0.2-overburn-cdr90.patch
# fix overburning check to not count the used capacity twice
# (followup to the above fix)
# https://projects.kde.org/projects/extragear/multimedia/k3b/repository/revisions/8270d2853f2962facdf6f9ea6d669922d6c78aba
Patch102: k3b-2.0.2-overburn-usedcapacity.patch

BuildRequires: desktop-file-utils
BuildRequires: gettext
BuildRequires: kdelibs4-devel
BuildRequires: kdemultimedia-devel
BuildRequires: libmpcdec-devel
BuildRequires: pkgconfig(dvdread)
BuildRequires: pkgconfig(flac++)
BuildRequires: pkgconfig(libmusicbrainz)
# needed by k3bsetup
#BuildRequires: pkgconfig(polkit-qt-1)

# Don't use WebKit on RHEL
%if 0%{?fedora}
BuildRequires: pkgconfig(QtWebKit)
%endif
BuildRequires: pkgconfig(samplerate)
BuildRequires: pkgconfig(sndfile)
BuildRequires: pkgconfig(taglib)
BuildRequires: pkgconfig(vorbisenc) pkgconfig(vorbisfile)
BuildRequires: pkgconfig(taglib)

Requires: kdebase-runtime%{?_kde_version: >= %{_kde4_version}}
Requires: %{name}-libs%{?_isa} = %{epoch}:%{version}-%{release}
Requires: %{name}-common = %{epoch}:%{version}-%{release}

Requires: cdrecord
Requires: cdrdao
Requires: dvd+rw-tools
Requires: mkisofs

%description
K3b provides a comfortable user interface to perform most CD/DVD
burning tasks. While the experienced user can take influence in all
steps of the burning process the beginner may find comfort in the
automatic settings and the reasonable k3b defaults which allow a quick
start.

%package common
Summary:  Common files of %{name}
Group:    Applications/Archiving
Requires: %{name} = %{epoch}:%{version}-%{release}
BuildArch: noarch
%description common
{summary}.

%package libs
Summary: Runtime libraries for %{name}
Group:   System Environment/Libraries
Requires: %{name} = %{epoch}:%{version}-%{release}
%{?_kde4_version:Requires: kdelibs4%{?_isa} >= %{_kde4_version}}
%{?_qt4_version:Requires: qt4%{?_isa} >= %{_qt4_version}}
%description libs
%{summary}.

%package devel
Summary: Files for the development of applications which will use %{name} 
Group: Development/Libraries
Requires: %{name}-libs%{?_isa} = %{epoch}:%{version}-%{release}
%description devel
%{summary}.


%prep
%setup -q -n %{name}-%{version}

%patch50 -p1 -b .use_vartmp_instead_of_tmp

%patch100 -p1 -b .use_growisofs_instead_of_wodim
%patch101 -p1 -b .overburn-cdr90
%patch102 -p1 -b .overburn-usedcapacity

%if 0%{?rhel} > 6
%patch1 -p1 -b .no-webkit
%endif


%build

mkdir -p %{_target_platform}
pushd %{_target_platform}
CXXFLAGS="$RPM_OPT_FLAGS -fno-strict-aliasing"
%{cmake_kde4} \
  -DK3B_BUILD_K3BSETUP:BOOL=OFF \
  -DK3B_BUILD_FFMPEG_DECODER_PLUGIN:BOOL=OFF \
  -DK3B_BUILD_LAME_ENCODER_PLUGIN:BOOL=OFF \
  -DK3B_BUILD_MAD_DECODER_PLUGIN:BOOL=OFF \
  -DK3B_ENABLE_HAL_SUPPORT:BOOL=OFF \
  ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
rm -rf %{buildroot}

make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

%find_lang k3b --with-kde
%find_lang kio_videodvd
%find_lang libk3b
%find_lang libk3bdevice 
cat kio_videodvd.lang >> k3b.lang
cat libk3bdevice.lang >> libk3b.lang

## unpackaged files
# k3bsetup crud
rm -fv %{buildroot}%{_datadir}/locale/*/LC_MESSAGES/k3bsetup.mo


%check
desktop-file-validate %{buildroot}%{_kde4_datadir}/applications/kde4/k3b.desktop


%clean
rm -rf %{buildroot}


%post libs -p /sbin/ldconfig

%postun libs -p /sbin/ldconfig

%post
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null || :
update-desktop-database -q &> /dev/null
update-mime-database %{_kde4_datadir}/mime &> /dev/null
fi

%posttrans
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null || :
update-desktop-database -q &> /dev/null
update-mime-database %{_kde4_datadir}/mime &> /dev/null


%files
%defattr(-,root,root,-)
%doc README COPYING COPYING.DOC ChangeLog
%{_kde4_bindir}/k3b
%{_kde4_libdir}/kde4/*.so
%{_kde4_appsdir}/konqsidebartng/virtual_folders/services/*.desktop
%{_kde4_appsdir}/solid/actions/k3b*.desktop
%{_kde4_datadir}/applications/kde4/k3b.desktop
%{_kde4_datadir}/kde4/services/*.desktop
%{_kde4_datadir}/kde4/services/*.protocol
%{_kde4_datadir}/kde4/services/ServiceMenus/k3b*.desktop
%{_kde4_datadir}/kde4/servicetypes/k3b*.desktop
%{_kde4_datadir}/mime/packages/x-k3b.xml
%{_kde4_iconsdir}/hicolor/*/*/*

%files common -f k3b.lang
%defattr(-,root,root,-)
%{_kde4_appsdir}/k3b/

%files libs -f libk3b.lang
%defattr(-,root,root,-)
%{_kde4_libdir}/libk3bdevice.so.6*
%{_kde4_libdir}/libk3blib.so.6*

%files devel
%defattr(-,root,root,-)
%{_kde4_includedir}/k3b*.h
%{_kde4_libdir}/libk3bdevice.so
%{_kde4_libdir}/libk3blib.so


%changelog
* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> - 1:2.0.2-17
- Mass rebuild 2013-12-27

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1:2.0.2-16
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Wed Dec 19 2012 Rex Dieter <rdieter@fedoraproject.org> 1:2.0.2-15
- use /var/tmp instead of /tmp

* Thu Nov 29 2012 Radek Novacek <rnovacek@redhat.com> 1:2.0.2-14
- Use -fno-strict-aliasing (k3b does some nasty stuff with pointers in iso9660 support)

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1:2.0.2-13
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Tue Jun 12 2012 Radek Novacek <rnovacek@redhat.com> 1:2.0.2-12
- Disable HAL support at all, it's not even in Fedora anymore

* Tue Jun 12 2012 Radek Novacek <rnovacek@redhat.com> 1:2.0.2-11
- Don't show suggestion to start HAL daemon when no CD/DVD device is found

* Fri May 04 2012 Radek Novacek <rnovacek@redhat.com> 1:2.0.2-10
- Fix conditional for RHEL

* Fri May 04 2012 Radek Novacek <rnovacek@redhat.com> 1:2.0.2-9
- Remove QtWebKit BR on RHEL

* Fri May 04 2012 Radek Novacek <rnovacek@redhat.com> 1:2.0.2-8
- Don't use webkit on RHEL

* Tue Feb 28 2012 Rex Dieter <rdieter@fedoraproject.org> 1:2.0.2-7
- pkgconfig-style deps
- +BR: pkgconfig(QtWebKit)

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1:2.0.2-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Sat Sep 17 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> - 1:2.0.2-5
- increase overburning tolerance from 1/10 to 1/4 for CD-R90/99 (kde#276002)
- fix overburning check to not count the used capacity twice

* Sat Apr 23 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> - 1:2.0.2-4
- prefer growisofs to wodim for DVD/BluRay also for DVD copying and data
  projects, the previous patch only worked for ISO burning (#610976)

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1:2.0.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Thu Jan 20 2011 Rex Dieter <rdieter@fedoraproject.org> - 1:2.0.2-2
- use growisofs for blueray too (#610976)

* Wed Jan 19 2011 Rex Dieter <rdieter@fedoraproject.org> - 1:2.0.2-1
- k3b-2.0.2 (#670325)
- use growisofs for dvd's (#610976)

* Tue Nov 23 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:2.0.1-5
- turns out existing hal support is mostly harmless (not required, but will
  use if present), so back out those changes.

* Tue Nov 23 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:2.0.1-4
- build without ENABLE_HAL_SUPPORT (f15+)
- spec cleanup

* Tue Nov 23 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:2.0.1-3
- Requires: hal-storage-addon

* Tue Oct 05 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:2.0.1-2
- drop workaround patch if building on/for kde-4.5.2+ (#582764)

* Tue Aug 31 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:2.0.1-1
- k3b-2.0.1

* Mon Aug 30 2010 Radek Novacek <rnovacek@redhat.com> - 1:2.0.0-3
- Added temporary workaround for #582764

* Tue Jul 06 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:2.0.0-2
- Missing Icon of k3b (#611272)

* Mon Jun 28 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:2.0.0-1
- k3b-2.0.0

* Thu Jun 17 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:1.93.0-1
- k3b-1.93.0 (rc3)

* Sat May 22 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:1.92.0-1
- k3b-1.92.0 (rc3)
- enable -common noarch content

* Sat Mar 13 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:1.91.0-2
- Requires: kdebase-runtime

* Sat Mar 13 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:1.91.0-1
- k3b-1.91.0 (rc2)

* Fri Mar 05 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:1.90.0-1
- k3b-1.90.0 (rc1)

* Mon Feb 01 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:1.70.0-1
- k3b-1.70.0 (beta1)

* Thu Jan 28 2010 Rex Dieter <rdieter@fedoraproject.org> - 1:1.69.0-3
- use %%{_kde4_version}

* Thu Dec 10 2009 Rex Dieter <rdieter@fedoraproject.org> - 1:1.69.0-2
- fix %%post scriptlet

* Thu Dec 10 2009 Rex Dieter <rdieter@fedoraproject.org> - 1:1.69.0-1
- k3b-1.69.0 (alpha4)

* Mon Nov 23 2009 Rex Dieter <rdieter@fedoraproject.org> - 1:1.68.0-3 
- rebuild (for qt-4.6.0-rc1, f13+)

* Thu Oct 22 2009 Rex Dieter <rdieter@fedoraproject.org> - 1:1.68.0-2
- -common (noarch) subpkg handling
- -libs: add min runtime deps for qt4/kdelibs4

* Thu Oct 15 2009 Rex Dieter <rdieter@fedoraproject.org> - 1:1.68.0-1
- k3b-1.68.0 (alpha3)

* Sat Sep 26 2009 Rex Dieter <rdieter@fedoraproject.org> - 1:1.66.0-5
- Epoch: 1 (increment Release too, to minimize confusion)

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0:1.66.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Thu Jun 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 0:1.66.0-3
- -extras-freeworld avail now, drop Obsoletes

* Wed Jun 17 2009 Rex Dieter <rdieter@fedoraproject.org> - 0:1.66.0-2
- Obsoletes: k3b-extras-freeworld (at least until it's ready)

* Wed May 27 2009 Rex Dieter <rdieter@fedoraproject.org> - 0:1.66.0-1
- k3b-1.66.0 (alpha2)

* Wed Apr 22 2009 Rex Dieter <rdieter@fedoraproject.org> - 0:1.65.0-1
- k3b-1.65.0 (alpha1)
- optimize scriptlets

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0:1.0.5-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Wed Jan 21 2009 Rex Dieter <rdieter@fedoraproject.org> - 0:1.0.5-7
- Summary: omit "for KDE"
- add rhel portability to .spec

* Wed Oct 01 2008 Rex Dieter <rdieter@fedoraproject.org> - 0:1.0.5-6
- revert libdvdread header changes, fix build (#465115)
- (re)enable -devel on f9

* Wed Aug 27 2008 Rex Dieter <rdieter@fedoraproject.org> - 0:1.0.5-5
- resurrect -devel (!=f9), grow -libs (f10+, #341651)
- avoid auto*foo
- fix build on rawhide (libdvdread header changes)
- conditionalize i18n bits

* Thu Jul 31 2008 Rex Dieter <rdieter@fedoraproject.org> - 0:1.0.5-4
- try alternative fix for tray eject/reload (kde#156684)

* Mon Jun 30 2008 Rex Dieter <rdieter@fedoraproject.org> - 0:1.0.5-3
- No association k3b with .iso files in gnome (#419681)
- scriptlet deps
- cleanup doc/HTML

* Sat May 31 2008 Rex Dieter <rdieter@fedoraproject.org> - 0:1.0.5-2
- (re)enable reload patch

* Tue May 27 2008 Rex Dieter <rdieter@fedoraproject.org> - 0:1.0.5-1
- k3b-1.0.5
- k3brc: set manual buffer size here
- omit reload patch (for now), to verify if still needed.

* Wed May  7 2008 Roman Rakus <rrakus@redhat.cz> - 0:1.0.4-9
- Fix doc dir (#238070), patch by Alain PORTAL (aportal@univ-montp2.fr)

* Tue Apr 22 2008 Roman Rakus <rrakus@redhat.cz> - 0:1.0.4-8
- Use manual buffer size by default (#220481)

* Tue Feb 19 2008 Rex Dieter <rdieter@fedoraproject.org> - 0:1.0.4-7
- f9+: Obsoletes: k3b-devel (#429613)

* Mon Feb 18 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 0:1.0.4-6
- Autorebuild for GCC 4.3

* Sat Dec 08 2007 Rex Dieter <rdieter[AT]fedoraproject.org> - 0:1.0.4-5
- patch for "k3b can't reload media for verification" (kde#151816)
- BR: kdelibs3-devel

* Wed Nov 21 2007 Adam Tkac <atkac redhat com> - 0:1.0.4-3
- rebuild against new libdvdread

* Mon Nov 05 2007 Rex Dieter <rdieter[AT]fedorproject.org> - 0:1.0.4-2
- k3b-1.0.4
- omit -devel subpkg (f9+), fixes multiarch conflicts (#341651)

* Fri Aug 17 2007 Harald Hoyer <harald@redhat.com> - 0:1.0.3-3
- changed license tag to GPLv2+

* Fri Aug  3 2007 Harald Hoyer <harald@redhat.com> - 0:1.0.3-2
- added gnome-umount options

* Fri Jul 27 2007 Harald Hoyer <harald@redhat.com> - 0:1.0.3-1
- version 1.0.3
- added gnome-umount patch

* Sat Jun 23 2007 Rex Dieter <rdieter[AT]fedoraproject.org> - 0:1.0.2-1
- k3b-1.0.2

* Sat Jun 16 2007 Rex Dieter <rdieter[AT]fedoraproject.org> - 0:1.0.1-4
- k3b-iso.desktop,k3b-cue.desktop: +NoDisplay=True (#244513)

* Wed Jun 13 2007 Rex Dieter <rdieter[AT]fedoraproject.org> - 0:1.0.1-3
- --without-cdrecord-suid-root

* Wed Jun 06 2007 Rex Dieter <rdieter[AT]fedoraproject.org> - 0:1.0.1-2
- respin (for libmpcdec)

* Wed May 30 2007 Rex Dieter <rdieter[AT]fedoraproject.org> - 0:1.0.1-1
- k3b-1.0.1
- include icon/mime scriptlets
- cleanup/simplify BR's
- optimize %%configure
- restore applnk/.hidden bits

* Wed Apr 11 2007 Harald Hoyer <harald@redhat.com> - 0:1.0-1
- version k3b-1.0
- provide/obsolete k3b-extras

* Thu Feb 15 2007 Harald Hoyer <harald@redhat.com> - 0:1.0.0-0.rc6.1
- version k3b-1.0rc6

* Wed Feb  7 2007 Harald Hoyer <harald@redhat.com> - 0:1.0.0-0.rc5.1
- version k3b-1.0rc5

* Wed Jan 17 2007 Harald Hoyer <harald@redhat.com> - 0:1.0.0-0.rc4.1
- version k3b-1.0rc4

* Thu Oct 26 2006 Harald Hoyer <harald@backslash.home> - 0:1.0.0-0.pre2.1
- version 1.0pre2

* Wed Jul 12 2006 Jesse Keating <jkeating@redhat.com> - 0:0.12.15-3.1.1
- rebuild

* Wed Jul 12 2006 Jesse Keating <jkeating@redhat.com> - 0:0.12.15-3.1
- rebuild

* Mon Jun 12 2006 Harald Hoyer <harald@redhat.com> - 0:0.12.15-3
- fixed symlinks

* Tue May 02 2006 Harald Hoyer <harald@redhat.com> 0:0.12.15-1
- version 0.12.15

* Fri Feb 10 2006 Jesse Keating <jkeating@redhat.com> - 0:0.12.10-2.2
- bump again for double-long bug on ppc(64)

* Tue Feb 07 2006 Jesse Keating <jkeating@redhat.com> - 0:0.12.10-2.1
- rebuilt for new gcc4.1 snapshot and glibc changes

* Tue Jan 24 2006 Harald Hoyer <harald@redhat.com> 0:0.12.10-2
- removed .la files (#172638)

* Tue Dec 20 2005 Harald Hoyer <harald@redhat.com> 0:0.12.10-1
- version 0.12.10

* Fri Dec 09 2005 Jesse Keating <jkeating@redhat.com>
- rebuilt

* Tue Dec 06 2005 Harald Hoyer <harald@redhat.com> 0:0.12.8-1
- version 0.12.8

* Wed Sep 21 2005 Harald Hoyer <harald@redhat.com> 0:0.12.4-0.a.1
- version 0.12.4a

* Thu Jul 14 2005 Harald Hoyer <harald@redhat.com> 0:0.12.2-1
- version 0.12.2
- ported some patches

* Wed Jul 11 2005 Harald Hoyer <harald@redhat.com> 0:0.11.23-2
- added "dvd+rw-tools cdrdao" to Requires

* Thu Apr  7 2005 Petr Rockai <prockai@redhat.com> - 0:0.11.23-3
- fix statfs usage (as per issue 65935 from IT)

* Wed Mar 30 2005 Harald Hoyer <harald@redhat.com> 0:0.11.23-1
- update to 0.11.23

* Fri Mar 25 2005 David Hill <djh[at]ii.net> 0:0.11.22-1
- update to 0.11.22

* Tue Mar 08 2005 Than Ngo <than@redhat.com> 0:0.11.17-2
- rebuilt against gcc-4

* Tue Oct 05 2004 Harald Hoyer <harald@redhat.com> 0:0.11.17-1
- version 0.11.17
- revert the dao -> tao change
- add the suid feature to every app automatically

* Tue Oct 05 2004 Harald Hoyer <harald@redhat.com> 0:0.11.14-2
- fixed version string parsing, which fixes bug 134642

* Wed Sep 01 2004 Harald Hoyer <harald@redhat.com> 0:0.11.14-1
- added k3b-0.11.14-rdrw.patch for kernel >= 2.6.8
- update to 0.11.14

* Fri Jun 25 2004 Bill Nottingham <notting@redhat.com> 0:0.11.12-2
- update to 0.11.12

* Mon Jun 21 2004 Than Ngo <than@redhat.com> 0:0.11.11-1
- update to 0.11.11
- add prereq:/sbin/ldconfig

* Tue Jun 15 2004 Elliot Lee <sopwith@redhat.com>
- rebuilt

* Mon May 31 2004 Justin M. Forbes <64bit_fedora@comcast.net> - 0.11.10-1
- remove unnecesary [ -z "$QTDIR" ] check
- Update to 0.11.10 upstream 
- remove qt-devel BuildRequires, implied with kde-devel
- remove ldconfig Requires, implied
- remove i18n docbook patch, fixed upstream.

* Fri May 28 2004 Bill Nottingham <notting@redhat.com> - 0.11.9-5
- fix burning on SCSI CD-ROMS (#122096)

* Thu May 13 2004 Than Ngo <than@redhat.com> 0.11.9-4
- get rid of rpath

* Fri Apr 16 2004 Bill Nottingham <notting@redhat.com> - 0.11.9-3
- nuke k3bsetup
- use %%find_lang

* Thu Apr 15 2004 Justin M. Forbes <64bit_fedora@comcast.net> - 0.11.9-2
- Clean up i18n build to make improve maintainability

* Wed Apr 7 2004 Justin M. Forbes <64bit_fedora@comcast.net> - 0.11.9-1
- Update to 0.11.9 upstream
- Spec Cleanup for Fedora Core 2

* Wed Mar 18 2004 Justin M. Forbes <64bit_fedora@comcast.net> - 0.11.6-1
- Initial packaging of 0.11.6 for Fedora Core 2
- remove mp3 plugin build options
- add i18n package
- clean up for kde 3.2/FC2 target


