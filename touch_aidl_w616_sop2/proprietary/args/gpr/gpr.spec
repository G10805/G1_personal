Summary: Generic Packet Router
Name: gpr
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
URL: http://support.cdmatech.com
Source0: %{name}-%{version}.tar.gz

%define DEPENDS glib2 ar-osal libglink-client
BuildRequires: cmake libtool gcc-g++ glib2-devel ar-osal-devel libglink-client-devel
Requires: %{DEPENDS}

%description
This is is a layer which route packet from GSL to SPF.

%package -n gpr-devel
Summary: Generic Packet Router - Development files
License: Qualcomm-Technologies-Inc.-Proprietary

%description -n gpr-devel
This is is a layer which route packet from GSL to SPF.
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
%{_libdir}/libgpr.so
%license NOTICE

%files -n gpr-devel
%{_includedir}/ar_guids.h
%{_includedir}/ar_msg.h
%{_includedir}/ar_types.h
%{_includedir}/gpr_api.h
%{_includedir}/gpr_api_i.h
%{_includedir}/gpr_api_inline.h
%{_includedir}/gpr_api_log.h
%{_includedir}/gpr_comdef.h
%{_includedir}/gpr_dynamic_allocation.h
%{_includedir}/gpr_glink.h
%{_includedir}/gpr_glink_i.h
%{_includedir}/gpr_heap_i.h
%{_includedir}/gpr_ids_domains.h
%{_includedir}/gpr_list.h
%{_includedir}/gpr_msg_if.h
%{_includedir}/gpr_packet.h
%{_includedir}/gpr_pack_begin.h
%{_includedir}/gpr_pack_end.h
%{_includedir}/gpr_private_api.h
%{_includedir}/gpr_proc_info_api.h
%{_includedir}/gpr_session.h
%{_includedir}/ipc_dl_api.h
%{_includedir}/private/gpr_private_api.h