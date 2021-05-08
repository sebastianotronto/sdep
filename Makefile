# See LICENSE file for copyright and license details.

VERSION = 0.1

PREFIX = ${HOME}/.local
MANPREFIX = ${PREFIX}/share/man
SDEPDATA = ${XDG_DATA_HOME}/sdep
SCRIPTS = sdep-add sdep-checknow sdep-clear sdep-edit sdep-list

CPPFLAGS = -D_XOPEN_SOURCE=700 -DVERSION=\"${VERSION}\"
CFLAGS   = -pedantic -Wall -Os ${CPPFLAGS}

CC = cc


all: options sdep

options:
	@echo sdep build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CC       = ${CC}"

sdep:
	${CC} ${CFLAGS} -o sdep sdep.c

clean:
	rm -f sdep sdep-${VERSION}.tar.gz

dist: clean
	mkdir -p sdep-${VERSION}
	cp -R LICENSE Makefile README sdep.1 sdep.c sdep-${VERSION}
	tar -cf sdep-${VERSION}.tar sdep-${VERSION}
	gzip sdep-${VERSION}.tar
	rm -rf sdep-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f sdep ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/sdep
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < sdep.1 > ${DESTDIR}${MANPREFIX}/man1/sdep.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/sdep.1

scripts:
	mkdir -p ${DESTDIR}${SDEPDATA}
	for s in ${SCRIPTS}; do\
		sed "s|SDEPDATA|${DESTDIR}${SDEPDATA}|g" < scripts/$$s \
		     > ${DESTDIR}${PREFIX}/bin/$$s ;\
		chmod 755 ${DESTDIR}${PREFIX}/bin/$$s ;\
	done

uninstall:
	rm -rf ${DESTDIR}${PREFIX}/bin/sdep ${DESTDIR}${MANPREFIX}/man1/sdep.1
	for s in ${SCRIPTS}; do rm -rf ${DESTDIR}${PREFIX}/bin/$$s; done

.PHONY: all options clean dist install scripts uninstall

