Summary: Graph Service Layer
Name: ar-gsl
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
URL: http://support.cdmatech.com
Source0: %{name}-%{version}.tar.gz

%define DEPENDS glib2 acdb ar-util gpr
BuildRequires: cmake libtool gcc-g++ spf-devel glib2-devel acdb-devel ar-util-devel gpr-devel
Requires: %{DEPENDS}

%description
This is the library used to setup and configure
the graph in SPF framework.

%package -n ar-gsl-devel
Summary: Graph Service Layer - Development files
License: Qualcomm-Technologies-Inc.-Proprietary

%description -n ar-gsl-devel
This is the library used to setup and configure
the graph in SPF framework.
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
%{_libdir}/libar_gsl.so

%files -n ar-gsl-devel
%{_includedir}/gsl_common.h
%{_includedir}/gsl_datapath.h
%{_includedir}/gsl_dynamic_module_mgr.h
%{_includedir}/gsl_global_persist_cal.h
%{_includedir}/gsl_graph.h
%{_includedir}/gsl_hw_rsc_intf.h
%{_includedir}/gsl_intf.h
%{_includedir}/gsl_main.h
%{_includedir}/gsl_mdf_utils.h
%{_includedir}/gsl_mdf_utils.xml
%{_includedir}/gsl_msg_builder.h
%{_includedir}/gsl_rtc.h
%{_includedir}/gsl_rtc_intf.h
%{_includedir}/gsl_rtc_main.h
%{_includedir}/gsl_shmem_mgr.h
%{_includedir}/gsl_spf_ss_state.h
%{_includedir}/gsl_subgraph.h
%{_includedir}/gsl_subgraph_driver_props.h
%{_includedir}/gsl_subgraph_driver_props.xml
%{_includedir}/gsl_subgraph_driver_props_generic.h
%{_includedir}/gsl_subgraph_platform_driver_props.xml
%{_includedir}/gsl_subgraph_pool.h