dirs = hello ring $(wildcard collectives/*)

default:
	$(foreach dir,$(dirs),$(MAKE) -C $(dir);)

clean:
	$(foreach dir,$(dirs),$(MAKE) -C $(dir) clean;)
