Name: NBIS          
Version: 4.1.0       
Release:        1%{?dist}
Summary: NIST Biometric Image Software       

License: Public Domain        
URL: http://www.nist.gov/itl/iad/ig/nbis.cfm           
Source0: %{name}-%{version}.tar.gz

%description


%prep
%setup -q

%build
unzip nbis_v4_1_0.zip
mkdir -p $RPM_BUILD_DIR/target
cd Rel_4.1.0
bash setup.sh $RPM_BUILD_DIR/target --64
make config
make it

%install
rm -rf $RPM_BUILD_ROOT
cd Rel_4.1.0
make install
mkdir -p $RPM_BUILD_ROOT%{_includedir}/nbis
mkdir -p $RPM_BUILD_ROOT%{_libdir}/nbis
mkdir -p $RPM_BUILD_ROOT%{_mandir}
mkdir -p $RPM_BUILD_ROOT%{_bindir}
cp -r $RPM_BUILD_DIR/target/include/* $RPM_BUILD_ROOT%{_includedir}/nbis/
cp -r $RPM_BUILD_DIR/target/lib/* $RPM_BUILD_ROOT%{_libdir}/nbis/
cp -r $RPM_BUILD_DIR/target/bin/* $RPM_BUILD_ROOT%{_bindir}
cp -r $RPM_BUILD_DIR/target/man/* $RPM_BUILD_ROOT%{_mandir}
#remove files that conflict with libjpeg-turbo
rm -f $RPM_BUILD_ROOT%{_bindir}/cjpeg
rm -f $RPM_BUILD_ROOT%{_mandir}/man1/cjpeg.1
rm -f $RPM_BUILD_ROOT%{_bindir}/djpeg
rm -f $RPM_BUILD_ROOT%{_mandir}/man1/djpeg.1
rm -f $RPM_BUILD_ROOT%{_bindir}/jpegtran
rm -f $RPM_BUILD_ROOT%{_mandir}/man1/jpegtran.1
rm -f $RPM_BUILD_ROOT%{_bindir}/rdjpgcom
rm -f $RPM_BUILD_ROOT%{_mandir}/man1/rdjpgcom.1
rm -f $RPM_BUILD_ROOT%{_bindir}/wrjpgcom
rm -f $RPM_BUILD_ROOT%{_mandir}/man1/wrjpgcom.1

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_includedir}/nbis/*
%{_libdir}/nbis/*
%{_bindir}/*
%{_mandir}/man1/*
%doc



%changelog
