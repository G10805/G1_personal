Summary: AudioReach Transport Service
Name: ats
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
URL: http://support.cdmatech.com
Source0: %{name}-%{version}.tar.gz

%define DEPENDS glib2 ar-osal acdb ar-gsl diag-lsm
BuildRequires: cmake libtool gcc-g++ glib2-devel ar-osal-devel acdb-devel ar-gsl-devel diag-lsm-devel
Requires: %{DEPENDS}

%description
This is the library used to transport service to QACT.

%package -n ats-devel
Summary: AudioReach Transport Service - Development files
License: Qualcomm-Technologies-Inc.-Proprietary

%description -n ats-devel
This is the library used to transport service to QACT.
This package contains symbolic links, header files
and related items necessary for software development.

%global debug_package %{nil}
%global __os_install_post %{nil}

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE -DCMAKE_RULE_MESSAGES:BOOL=OFF -DCMAKE_VERBOSE_MAKEFILE:BOOL=$VER -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir} -DCMAKE_INSTALL_LIBDIR=%{_libdir} -DCMAKE_INSTALL_BINDIR=%{_bindir} -DCMAKE_INSTALL_LICENSEDIR=%{_defaultlicensedir}
%cmake_build

%install
%cmake_install

%files
%{_libdir}/libats.so

%files -n ats-devel
%{_includedir}/actp.h
%{_includedir}/adie_rtc_api.h
%{_includedir}/ats.h
%{_includedir}/ats_adie_rtc.h
%{_includedir}/ats_command.h
%{_includedir}/ats_common.h
%{_includedir}/ats_fts.h
%{_includedir}/ats_i.h
%{_includedir}/ats_mcs.h
%{_includedir}/ats_online.h
%{_includedir}/ats_rtc.h
%{_includedir}/ats_server.h
%{_includedir}/audtp.h
%{_includedir}/audtpi.h
%{_includedir}/mcs_api.h
