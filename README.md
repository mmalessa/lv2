# Sandbox with my LV2 plugins

## Requirements
```
sudo apt-get install build-essential
sudo apt-get install pkg-config lv2-dev libsndfile1-dev libx11-dev libcairo2-dev
```

## Kick start
Go to plugin folder and...
```
make
make instal
make run-jalv
make clean
make uninstall
```
*`make install` installs the plugin in the directory `~/.lv2/$(PLUGIN_NAME)`

## Cross-references
- https://lv2plug.in/book/
- http://lv2plug.in/ns/
- https://msic.neocities.org/
- https://github.com/lv2/lv2
- https://github.com/sjaehn/lv2tutorial
