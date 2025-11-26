Summary: Audio Calibration Database
Name: acdb
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
URL: http://support.cdmatech.com
Source0: %{name}-%{version}.tar.gz

%define DEPENDS ar-osal
BuildRequires: cmake libtool gcc-g++ ar-osal-devel
Requires: %{DEPENDS}

%description
This is a library that provides audio calibration
capability to its clients.

%package -n acdb-devel
Summary: Audio Calibration Database - Development files
License: Qualcomm-Technologies-Inc.-Proprietary

%description -n acdb-devel
This is a library that provides audio calibration
capability to its clients.
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
%{_libdir}/libacdb.so

%files -n acdb-devel
%{_includedir}/acdb.h
%{_includedir}/acdb_begin_pack.h
%{_includedir}/acdb_command.h
%{_includedir}/acdb_common.h
%{_includedir}/acdb_data_proc.h
%{_includedir}/acdb_delta_file_mgr.h
%{_includedir}/acdb_delta_parser.h
%{_includedir}/acdb_end_pack.h
%{_includedir}/acdb_file_mgr.h
%{_includedir}/acdb_heap.h
%{_includedir}/acdb_init.h
%{_includedir}/acdb_init_utility.h
%{_includedir}/acdb_parser.h
%{_includedir}/acdb_utility.h