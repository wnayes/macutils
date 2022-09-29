BINDIR ?=	bin
# Use the following flags on the CF macro definition as needed.
#
# -DBSD if you are on a BSD system
#
# -DTYPES_H if your system has /usr/include/sys/types.h
#
# -DDIRENT_H if your system has /usr/include/dirent.h
#
# -DTERMIOS_H if your system has /usr/include/sys/termios.h
#
# -DNODOT if you do not want to create files with an initial period
#
# -DLATIN1 if your system supports LATIN-1 and you want to use it
#
# Note you can use at most one of the following four!
#
# -DNOMKDIR if your system does not have the mkdir system call
#
# -DAUFS if you want to use an AUFS file system
#
# -DAUFSPLUS if you use CAP 6.0 and want to use times on files
#
# -DAPPLEDOUBLE if you want to be able to use an AppleDouble file system
#
CF ?=	-DBSD -DTYPES_H -DDIRENT_H -DTERMIOS_H -DNODOT -DAPPLEDOUBLE

all:
	(cd crc; $(MAKE) CF='$(CF)')
	(cd util; $(MAKE) CF='$(CF)')
	(cd fileio; $(MAKE) CF='$(CF)')
	(cd macunpack; $(MAKE) CF='$(CF)')
	(cd hexbin; $(MAKE) CF='$(CF)')
	(cd mixed; $(MAKE) CF='$(CF)')
	(cd binhex; $(MAKE) CF='$(CF)')
	(cd comm; $(MAKE) CF='$(CF)')

clean:
	(cd crc; $(MAKE) clean)
	(cd util; $(MAKE) clean)
	(cd fileio; $(MAKE) clean)
	(cd macunpack; $(MAKE) clean)
	(cd hexbin; $(MAKE) clean)
	(cd mixed; $(MAKE) clean)
	(cd binhex; $(MAKE) clean)
	(cd comm; $(MAKE) clean)

clobber:
	(cd crc; $(MAKE) clean)
	(cd util; $(MAKE) clean)
	(cd fileio; $(MAKE) clean)
	(cd macunpack; $(MAKE) clobber)
	(cd hexbin; $(MAKE) clobber)
	(cd mixed; $(MAKE) clobber)
	(cd binhex; $(MAKE) clobber)
	(cd comm; $(MAKE) clobber)

lint:
	(cd macunpack; $(MAKE) CF='$(CF)' lint)
	(cd hexbin; $(MAKE) CF='$(CF)' lint)
	(cd mixed; $(MAKE) CF='$(CF)' lint)
	(cd binhex; $(MAKE) CF='$(CF)' lint)
	(cd comm; $(MAKE) CF='$(CF)' lint)

install:
	mkdir -p "$(BINDIR)"
	install macunpack/macunpack "$(BINDIR)/."
	install hexbin/hexbin "$(BINDIR)/."
	install mixed/macsave "$(BINDIR)/."
	install mixed/macstream "$(BINDIR)/."
	install binhex/binhex "$(BINDIR)/."
	install comm/tomac "$(BINDIR)/."
	install comm/frommac "$(BINDIR)/."

distr:
	shar -a README makefile crc util fileio macunpack hexbin mixed binhex  \
		comm doc man >macutil.shar

