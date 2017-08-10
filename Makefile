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

clean:
	cd src && $(MAKE) clean
	cd mod && $(MAKE) clean
	cd tex && $(MAKE) clean
