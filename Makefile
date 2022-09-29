INCLUDE=-Iinclude
LIB=-Lbin
.PHONY: test install uninstall clean

bin/libfrequests.a:temp/frequests.o
	ar rcsv $@ $^
temp/frequests.o:src/frequests.cpp
	g++ $(INCLUDE) -c $^  -o $@

test:
	$(MAKE) -C tests
clean:
	$(RM) bin/*
	$(RM) temp/*