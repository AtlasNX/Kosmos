export APP_VERSION	:=	3.0.1

ifeq ($(RELEASE),)
	export APP_VERSION	:=	$(APP_VERSION)-$(shell git describe --dirty --always)
endif

.PHONY: clean all nx pc dist-bin

all: nx pc

romfs	:	assets
	@mkdir -p romfs
	@rm -f romfs/assets.zip
	@zip -rj romfs/assets.zip assets

dist-bin:	romfs
	$(MAKE) -f Makefile.nx dist-bin

nx:	romfs
	$(MAKE) -f Makefile.nx

pc:	romfs
	$(MAKE) -f Makefile.pc

clean:
	@rm -Rf romfs
	$(MAKE) -f Makefile.pc clean
	$(MAKE) -f Makefile.nx clean
