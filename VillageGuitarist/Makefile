-include ../.env.local

PLUGIN_NAME ?= VillageGuitarist
TARGET_DIRECTORY ?= ~/.lv2/$(PLUGIN_NAME).lv2
RUN_TEST_SESSION_COMMAND ?= /opt/Ardour-6.9.0/bin/ardour6 ~/Sessions/my_test_session.ardour

all:
	g++ -fvisibility=hidden \
		-fPIC \
		-Wl,-Bstatic -Wl,-Bdynamic -Wl,--as-needed \
		-shared \
		-pthread `pkg-config --cflags lv2` \
		-lm `pkg-config --libs lv2` \
		$(PLUGIN_NAME).cpp -o $(PLUGIN_NAME).so

clean:
	rm $(PLUGIN_NAME).so

install:
	[ -d $(TARGET_DIRECTORY) ] || mkdir $(TARGET_DIRECTORY)
	cp $(PLUGIN_NAME).so $(TARGET_DIRECTORY)/
	cp $(PLUGIN_NAME).ttl $(TARGET_DIRECTORY)/
	cp manifest.ttl $(TARGET_DIRECTORY)/

uninstall:
	[ -d $(TARGET_DIRECTORY) ] && rm -rf $(TARGET_DIRECTORY)

run-jalv:
	jalv.gtk3 https://github.com/mmalessa/lv2/$(PLUGIN_NAME)

run-test-session:
	$(RUN_TEST_SESSION_COMMAND)