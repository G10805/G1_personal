Summary: FastCV sample applications
Name: fastcv
Version: 1.0
Release: 1%{?dist}
License: Qualcomm-Technologies-Inc.-Proprietary
Group: multimedia
URL: http://support.cdmatech.com
Source0: %{name}-%{version}.tar.gz
BuildRequires: autoconf automake libtool gcc-g++ liblog-dev libcutils-dev fastcv-prop-devel fastcv-prop fastrpc rpcmem glib2-devel
Requires: fastrpc fastcv-prop glib2

%description
FastCV sample applications

%define fastcvBuildPath %{_buildrootdir}/%{name}-%{version}-%{release}.%{_arch}

%prep
# nothing to prep

%setup -qn %{name}

%build
autoreconf -if
%configure --enable-arm-native-compilation=yes
%make_build

%install
%make_install

mkdir -p %{fastcvBuildPath}/usr/share/fastrpc/
%{__install} -D -m644 ./samples/nsp-simple-test/skel/lib/libfastcvNSPSimpleTest_skel.so %{fastcvBuildPath}/usr/share/fastrpc/

%files
%{_bindir}/fastcv_simple_test
%{_bindir}/fastcv_nsp_simple_test
%{_bindir}/fastcv_hetero_compute_test
%{_usr}/share/fastrpc/libfastcvNSPSimpleTest_skel.so
