Name: NBIS          
Version: 4.1.0       
Release:        5%{?dist}
Summary: NIST Biometric Image Software       

License: Public Domain        
URL: http://www.nist.gov/itl/iad/ig/nbis.cfm           
Source0: %{name}-%{version}.tar.gz

%description


%prep
%setup -q

%build
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
* Wed Mar 27 2013 Aaron Donovan <amdonov@gmail.com> 4.1.0-5
- Corrected error with replacement of static variables (amdonov@gmail.com)
- Updating autocrop to use new wsq API (amdonov@gmail.com)
- Working to make wsq library thread safe. At this point it doesn't have
  read/write globals or statics, but it doesn't work. (amdonov@gmail.com)

* Sun Mar 24 2013 Aaron Donovan <amdonov@gmail.com> 4.1.0-4
- Minor changes to all bozorth3 command line tool to deal with changes to the
  library (amdonov@gmail.com)

* Sun Mar 24 2013 Aaron Donovan <amdonov@gmail.com> 4.1.0-3
- Cleaning up if/else (amdonov@gmail.com)

* Sun Mar 24 2013 Aaron Donovan <amdonov@gmail.com> 4.1.0-2
- Heavy refactoring of bozorth code to eliminate global and static varibles.
  Matching is now thread-safe. No memory leaks on the default execution path,
  but some resources may still be improperly freed in certain situations.
  (amdonov@gmail.com)
- Formatted source files with indent (amdonov@gmail.com)
- Expanding source because bozorth3 requires extensive patching to make it
  thread safe (amdonov@gmail.com)

* Mon Mar 18 2013 Aaron Donovan <amdonov@gmail.com> 4.1.0-1
- new package built with tito

