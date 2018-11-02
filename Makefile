default: framework examples

framework:
	$(MAKE) -C src

docs:
	$(MAKE) -C docs
	
#examples: framework
#	$(MAKE) -C examples

.PHONY: clean default framework docs framework-clean

clean: framework-clean
	@-rm src/libexampi.a	

framework-clean:
	$(MAKE) -C src clean

#examples-clean:
#	$(MAKE) -C examples clean
