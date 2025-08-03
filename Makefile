CC = xcrun -sdk iphoneos clang
STRIP = xcrun -sdk iphoneos strip
LDFLAGS = -framework CoreFoundation -framework IOKit
32_LDFLAGS = $(LDFLAGS)
32_LDFLAGS += -miphoneos-version-min=6.0
LDFLAGS += -miphoneos-version-min=11.0

.PHONY: clean

restored_external: src/restored_external_hax.c
	$(CC) -arch arm64 -Wall $^ $(LDFLAGS) -o $@
	$(STRIP) $@
	@ldid -Srestored_external.plist restored_external

restored_external_32: src/restored_external_hax.c
	$(CC) -arch armv7 -Wall $^ $(32_LDFLAGS) -o $@
	$(STRIP) $@
	# @ldid -Srestored_external.plist restored_external_32

clean:
	rm restored_external
