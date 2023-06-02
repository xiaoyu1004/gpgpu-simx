all:
	$(MAKE) -C third_party
	$(MAKE) -C simx
	$(MAKE) -C driver
	$(MAKE) -C runtime
	$(MAKE) -C tests

clean:
	$(MAKE) -C third_party clean
	$(MAKE) -C simx clean
	$(MAKE) -C driver clean
	$(MAKE) -C runtime clean
	$(MAKE) -C tests clean