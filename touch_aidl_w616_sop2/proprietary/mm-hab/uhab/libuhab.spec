Summary: libuhab is for those who want to use HAB in the user space
Name: libuhab
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
Source: %{name}-%{version}.tar.gz
BuildRequires:	make libtool gcc-g++ systemd-rpm-macros liblog-dev pkgconfig pkgconfig(glib-2.0)

%description
HAB(Hypervisor ABstraction) can be used for the communication between the
GVM(Guest Virtual Machine) and host. Here, the libuhab is a wrapper of the
kernel space driver.

%prep
%autosetup -n uhab

%global debug_package %{nil}

%build
%set_build_flags

$CC -D__linux__ -Dstrlcat=g_strlcat -Dstrlcpy=g_strlcpy -c -fPIC uhab.c
$CC -shared -Wl,--no-undefined -o libuhab.so uhab.o -lglib-2.0 

%install
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_includedir}
mkdir -p  "$RPM_BUILD_ROOT/usr/lib/modules-load.d"
cp libuhab.so %{buildroot}%{_libdir}/libuhab.so
cp habmm.h %{buildroot}%{_includedir}/habmm.h

%files
%{_libdir}/libuhab.so
%{_includedir}/habmm.h
