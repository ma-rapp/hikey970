#PROJECTS = perfcounters perfcounters_csv global_perfcounters temperature cci_perfcounters
PROJECTS = perfcounters_csv
MODULES = cci_pmu
SHELL := /bin/bash

all:
	$(foreach p,$(PROJECTS), mkdir -p $(p)/native $(p)/hikey || exit;)
	$(foreach p,$(PROJECTS), cd $(CURDIR)/$(p)/native && cmake .. && make|| exit;)
	$(foreach p,$(PROJECTS), cd $(CURDIR)/$(p)/hikey && cmake .. -DCMAKE_TOOLCHAIN_FILE=../../toolchain_hikey.cmake && make || exit;)
#	$(foreach m,$(MODULES), make -C $(CURDIR)/modules/$(m);)
#	for PROJECT in $(PROJECTS) ; do \
#		mkdir -p $$PROJECT/native $$PROJECT/hikey && \
#		cd $(CURDIR)/$$PROJECT/native && cmake .. && make && \
#		cd $(CURDIR)/$$PROJECT/hikey && cmake .. -DCMAKE_TOOLCHAIN_FILE=../../toolchain_hikey.cmake && make; \
#	done

clean:
	for PROJECT in $(PROJECTS) ; do \
		rm -rf $$PROJECT/native $$PROJECT/hikey; \
	done

.PHONY: all clean
