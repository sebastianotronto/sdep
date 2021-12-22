# See LICENSE file for copyright and license details.

VERSION = 0.1

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
SDEPDATA = ${HOME}/.local/share/sdep
SCRIPTS = sdep-add sdep-checknow sdep-clear sdep-edit sdep-list sdep-checkpast

CPPFLAGS = -D_XOPEN_SOURCE=700 -DVERSION=\"${VERSION}\"
CFLAGS   = -pedantic -Wall -Os ${CPPFLAGS}

CC = cc


all: options sdep

options:
	@echo sdep build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CC       = ${CC}"

sdep:
	${CC} ${CFLAGS} -o sdep.o sdep.c

clean:
	rm -rf sdep.o sdep-${VERSION}.tar.gz scripts-build

dist: clean
	mkdir -p sdep-${VERSION}
	cp -R LICENSE Makefile README sdep.1 sdep.c sdep-${VERSION}
	tar -cf sdep-${VERSION}.tar sdep-${VERSION}
	gzip sdep-${VERSION}.tar
	rm -rf sdep-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f sdep.o ${DESTDIR}${PREFIX}/bin/sdep
	chmod 755 ${DESTDIR}${PREFIX}/bin/sdep
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < sdep.1 \
				     > ${DESTDIR}${MANPREFIX}/man1/sdep.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/sdep.1

scripts:
	mkdir -p scripts-build
	mkdir -p ${SDEPDATA}
	for s in ${SCRIPTS}; do\
		sed "s|SDEPDATA|${SDEPDATA}|g" < scripts/$$s \
		                               > scripts-build/$$s ;\
	done

scriptsinstall:
	for s in ${SCRIPTS}; do\
		cp -f scripts-build/$$s ${DESTDIR}${PREFIX}/bin ; \
		chmod 755 ${DESTDIR}${PREFIX}/bin/$$s ;\
	done

uninstall:
	rm -rf ${DESTDIR}${PREFIX}/bin/sdep ${DESTDIR}${MANPREFIX}/man1/sdep.1
	for s in ${SCRIPTS}; do rm -rf ${DESTDIR}${PREFIX}/bin/$$s; done

.PHONY: all options clean dist install scripts uninstall

