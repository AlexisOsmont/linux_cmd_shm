tar:
	$(MAKE) -C demon clean
	tar -zcf "$(CURDIR).tar.gz" client/* demon/* thread_pool/* tubes_names/* MACROS.h rapport.pdf README.pdf makefile
