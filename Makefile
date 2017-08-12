default: framework modules examples

framework:
	$(MAKE) -C src

modules:
	$(MAKE) -C mod

docs:
	$(MAKE) -C tex
	
examples: framework modules
	$(MAKE) -C examples

.PHONY: clean

clean: framework-clean modules-clean examples-clean

framework-clean:
	$(MAKE) -C src clean

modules-clean:
	$(MAKE) -C mod clean

examples-clean:
	$(MAKE) -C examples clean
