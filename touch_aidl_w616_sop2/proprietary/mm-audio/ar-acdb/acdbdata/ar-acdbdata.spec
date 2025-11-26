Summary: Audio Calibration Data
Name: ar-acdbdata
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
URL: http://support.cdmatech.com
Source0: %{name}-%{version}.tar.gz
BuildRequires: autoconf automake libtool gcc-g++

%description
These are audio calibration database files.

%package -n ar-acdbdata-devel
Summary: Audio Calibration Data - Development files
License: Qualcomm-Technologies-Inc.-Proprietary

%description -n ar-acdbdata-devel
These are audio calibration database files.
This package contains symbolic links, header files
and related items necessary for software development.

%global debug_package %{nil}
%global __os_install_post %{nil}

%prep
%autosetup -n %{name}-%{version}

%build
autoreconf -if
%configure
%make_build

%define ACDB_SRC_DIR ADP_AR/LRH
%define ACDB_DEST_DIR acdbdata/ADP_AR

%if "%{BASEMACHINE}" == "sa8775"
    %define TOPLEVEL_DIR lemans_au
%elif "%{BASEMACHINE}" == "sa8540" || "%{BASEMACHINE}" == "sa8295"
    %define TOPLEVEL_DIR makena_au
%endif

%install
%make_install
mkdir -p %{buildroot}/%{_sysconfdir}/%{ACDB_DEST_DIR}/
%if 0%{?TOPLEVEL_DIR:1}
    echo "TOPLEVEL_DIR is defined with value %{TOPLEVEL_DIR}"
    install -m 0644 %{TOPLEVEL_DIR}/%{ACDB_SRC_DIR}/acdb_cal.acdb %{buildroot}/%{_sysconfdir}/%{ACDB_DEST_DIR}/acdb_cal.acdb
    install -m 0644 %{TOPLEVEL_DIR}/%{ACDB_SRC_DIR}/workspaceFile.qwsp %{buildroot}/%{_sysconfdir}/%{ACDB_DEST_DIR}/workspaceFile.qwsp
    install -m 0644 %{TOPLEVEL_DIR}/%{ACDB_SRC_DIR}/acdb_cal.acdbdelta %{buildroot}/%{_sysconfdir}/%{ACDB_DEST_DIR}/acdb_cal.acdbdelta
%else
    echo "TOPLEVEL_DIR is not defined"
%endif

%post
chattr +i /etc/acdbdata/ADP_AR/acdb_cal.acdbdelta

%files
%{_libdir}/libacdbdata.so
%{_sysconfdir}/%{ACDB_DEST_DIR}/acdb_cal.acdb
%{_sysconfdir}/%{ACDB_DEST_DIR}/workspaceFile.qwsp
%{_sysconfdir}/%{ACDB_DEST_DIR}/acdb_cal.acdbdelta

%files -n ar-acdbdata-devel
%{_libdir}/pkgconfig/acdbdata.pc

%exclude %{_libdir}/libacdbdata.la
