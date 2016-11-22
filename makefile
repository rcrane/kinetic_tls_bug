SHELL = /bin/bash


SSLINC="openssl-gcc/target/include"
KININC="kinetic-c-gcc/include"

SSLLD="openssl-gcc/target/lib"
KINLD="kinetic-c-gcc/bin"
JSONLD="kinetic-c-gcc/vendor/json-c/.libs"


KININC2="kinetic-c-patched/include"
KINLD2 ="kinetic-c-patched/bin"
JSONLD2="kinetic-c-patched/vendor/json-c/.libs"


CFLAGS += -Wall -Wfatal-errors -std=gnu99 -I$(SSLINC) 

LDFLAGS += -L$(SSLLD) -lkinetic-c-client.0.12.0 -lpthread -lssl -ljson-c -lcrypto -ldl


ifndef DEBUG
	CFLAGS += -O3 -DCS_DISABLE_STDIO -DCS_NDEBUG 
else
	CFLAGS += -g -ggdb3 -pedantic -fsanitize=address
endif


SOURCES=$(wildcard *.c)

all: $(SOURCES)
	$(CC) -v -o kintest $(SOURCES) $(CFLAGS) -I$(KININC) -L$(KINLD) -L$(JSONLD) $(LDFLAGS)
	$(CC) -v -o kintest_patched $(SOURCES) $(CFLAGS) -I$(KININC2) -L$(KINLD2) -L$(JSONLD2) $(LDFLAGS)

clean:
	$(RM) -f kintest kintest_patched *.log


rebuild: clean all
