PKG = MDXapc
PKG_VERSION = v0.1

PKGFILE = $(PKG)-$(PKG_VERSION)-sparc.pkg

CC = /opt/SUNWspro/bin/cc
LD = /usr/ccs/bin/ld

CFLAGS = -D_KERNEL #-DDEBUG
LDFLAGS = -r

PKGMK = /bin/pkgmk
PKGTRANS = /bin/pkgtrans


all: $(PKGFILE)


apc: apc.c
	$(CC) $(CFLAGS) -c apc.c
	$(LD) $(LDFLAGS) -o apc apc.o


$(PKGFILE): apc pkginfo prototype LICENSE postinstall postremove
	$(PKGMK) -o -d .
	$(PKGTRANS) -s . $(PKGFILE) $(PKG)


clean:
	rm -f apc apc.o
	rm -rf $(PKG) $(PKGFILE)
