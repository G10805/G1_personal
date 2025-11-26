Summary: uhabtest is for those who want to test HAB in the user space
Name: uhabtest
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
Source: %{name}-%{version}.tar.gz
BuildRequires:	make libtool gcc-g++ systemd-rpm-macros liblog-dev libuhab

%description
HAB(Hypervisor ABstraction) can be used for the communication between the
GVM(Guest Virtual Machine) and host.

%prep
%autosetup -n testapp

%build
%set_build_flags

${CC} ${CFLAGS} -D__linux__ -DTEST_DMAHEAP_EXPORT -DCONFIG_HGY_PLATFORM -o uhabtest habtest.c habmenu.c habtestmem.c khab_test.c -lm -lpthread -luhab

%install
mkdir -p %{buildroot}%{_bindir}
cp uhabtest %{buildroot}%{_bindir}/uhabtest

chmod +x %{buildroot}%{_bindir}/uhabtest

%files
%{_bindir}/uhabtest

