default: framework modules docs

framework:
	$(MAKE) -C src

modules:
	$(MAKE) -C mod

docs:
	$(MAKE) -C tex
	
.PHONY: clean

clean:
	cd src && $(MAKE) clean
	cd mod && $(MAKE) clean
	cd tex && $(MAKE) clean
