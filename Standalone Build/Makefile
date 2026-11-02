# Standalone builds. Run from this folder and set OQS to the liboqs build or install prefix, for example:
#   cd standalone_build && make OQS=/path/to/liboqs/build
OQS    ?= /usr/local
# Add LDEXTRA=-lcrypto if liboqs was built with OpenSSL as a static library
LDEXTRA ?=
CC     ?= gcc
CFLAGS ?= -O2 -std=gnu11
INC     = -I../auth -I$(OQS)/include
LIBS    = -L$(OQS)/lib -Wl,-rpath,$(OQS)/lib -loqs -lm -lpthread $(LDEXTRA)
AUTH    = ../auth/vehicle.c ../auth/authentication.c ../auth/session_key.c ../auth/asra.c
OUT     = bin

all: $(OUT)/v2v_demo $(OUT)/calibrate $(OUT)/ml_dsa_test $(OUT)/ml_kem_test

$(OUT):
	mkdir -p $(OUT)
$(OUT)/v2v_demo: ../auth/main.c $(AUTH) | $(OUT)
	$(CC) $(CFLAGS) $(INC) $^ $(LIBS) -o $@
$(OUT)/calibrate: ../auth/calibrate.c $(AUTH) | $(OUT)
	$(CC) $(CFLAGS) $(INC) $^ $(LIBS) -o $@
$(OUT)/ml_dsa_test: ../dsa/ml_dsa_test.c | $(OUT)
	$(CC) $(CFLAGS) $(INC) $^ $(LIBS) -o $@
$(OUT)/ml_kem_test: ../kem/ml_kem_test.c | $(OUT)
	$(CC) $(CFLAGS) $(INC) $^ $(LIBS) -o $@

clean:
	rm -rf $(OUT)

.PHONY: all clean
