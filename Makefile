default: framework modules examples

framework:
	$(MAKE) -C src

modules:
	$(MAKE) -C mod

docs:
	$(MAKE) -C tex
	
examples: framework modules
	$(MAKE) -C examples

.PHONY: clean default framework modules docs feamework-clean modules-clean examples-clean

clean: framework-clean modules-clean examples-clean
	@-rm src/libexampi.a	

framework-clean:
	$(MAKE) -C src clean

modules-clean:
	$(MAKE) -C mod clean

examples-clean:
	$(MAKE) -C examples clean
