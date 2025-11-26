Summary: AudioReach Util library
Name: ar-util
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
URL: http://support.cdmatech.com
Source0: %{name}-%{version}.tar.gz

%define DEPENDS glib2 ar-osal
BuildRequires: cmake libtool gcc-g++ glib2-devel ar-osal-devel
Requires: %{DEPENDS}

%description
This is the library used to define public
AudioReach util APIs for double linked list.

%package -n ar-util-devel
Summary: AudioReach Util library - Development files
License: Qualcomm-Technologies-Inc.-Proprietary

%description -n ar-util-devel
This is the library used to define public
AudioReach util APIs for double linked list.
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
%{_libdir}/libar_util.so

%files -n ar-util-devel
%{_includedir}/ar_util_data_log.h
%{_includedir}/ar_util_data_log_codes.h
%{_includedir}/ar_util_list.h
%{_includedir}/ar_util_log_pkt_i.h