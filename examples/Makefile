dirs = hello ring

default:
	$(foreach dir,$(dirs),$(MAKE) -C $(dir);)

clean:
	$(foreach dir,$(dirs),$(MAKE) -C $(dir) clean;)
