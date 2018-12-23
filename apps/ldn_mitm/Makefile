KIPS := ldn_mitm
NROS := ldnmitm_debug

SUBFOLDERS := libstratosphere $(KIPS) $(NROS)

TOPTARGETS := all clean

$(TOPTARGETS): $(SUBFOLDERS)

$(SUBFOLDERS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(KIPS): libstratosphere

.PHONY: $(TOPTARGETS) $(SUBFOLDERS)
