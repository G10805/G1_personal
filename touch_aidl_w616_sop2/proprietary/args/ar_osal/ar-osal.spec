Summary: AudioReach OSAL (Operating System Abstracting Layer)
Name: ar-osal
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
URL: http://support.cdmatech.com
Source0: %{name}-%{version}.tar.gz

%define DEPENDS libcutils libkiumd diag-lsm
BuildRequires: cmake libtool gcc-g++ audio-headers-export glib2-devel libkiumd-dev %{DEPENDS} audio-utils-devel diag-lsm-devel
Requires: glib2 %{DEPENDS} audio-utils

%description
This is the AudioReach OSAL library which provides task scheduling,
time management and other functions for AudioReach.

%package -n ar-osal-devel
Summary: AudioReach OSAL (Operating System Abstracting Layer) - Development files
License: Qualcomm-Technologies-Inc.-Proprietary

%description -n ar-osal-devel
This is the AudioReach OSAL library which provides task scheduling,
time management and other functions for AudioReach.
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
%{_libdir}/libar_osal.so

%files -n ar-osal-devel
%{_includedir}/ar_osal_error.h
%{_includedir}/ar_osal_file_io.h
%{_includedir}/ar_osal_heap.h
%{_includedir}/ar_osal_log.h
%{_includedir}/ar_osal_log_pkt_op.h
%{_includedir}/ar_osal_mem_op.h
%{_includedir}/ar_osal_mutex.h
%{_includedir}/ar_osal_servreg.h
%{_includedir}/ar_osal_shmem.h
%{_includedir}/ar_osal_signal.h
%{_includedir}/ar_osal_signal2.h
%{_includedir}/ar_osal_sleep.h
%{_includedir}/ar_osal_string.h
%{_includedir}/ar_osal_sys_id.h
%{_includedir}/ar_osal_thread.h
%{_includedir}/ar_osal_timer.h
%{_includedir}/ar_osal_types.h
