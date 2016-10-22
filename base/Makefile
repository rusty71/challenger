ARDUINO_PATH = /usr/local/share/arduino/
SKETCHBOOK   =  $(CURDIR)
SKETCH       = base
TARGET_DIR   = $(SKETCHBOOK)/build
MONITOR_PORT = /dev/ttyUSB1

all:
	@ mkdir -p $(TARGET_DIR)

	$(ARDUINO_PATH)/arduino-builder -compile -logger=machine \
	-hardware "$(ARDUINO_PATH)/hardware" \
	-hardware "$(SKETCHBOOK)/hardware" \
	-hardware "/home/jeroen/.arduino15/packages" \
	-tools "$(ARDUINO_PATH)/tools-builder" \
	-tools "$(ARDUINO_PATH)/hardware/tools/avr" \
	-built-in-libraries "$(ARDUINO_PATH)/libraries" \
	-libraries "$(SKETCHBOOK)/lib" \
	-libraries "/home/jeroen/dev/moteino/challenger/lib" \
	-fqbn=Moteino:avr:MoteinoMEGA \
	-ide-version=10606 \
	-build-path "$(TARGET_DIR)" \
	-warnings=none \
	-prefs=build.warn_data_percentage=75 \
	-verbose "src/$(SKETCH).ino"

flash:
	avrdude -C/usr/local/share/arduino-1.6.9/hardware/tools/avr/etc/avrdude.conf -carduino -patmega1284p -P/dev/ttyUSB1 -b115200 -D -Uflash:w:build/$(SKETCH).ino.hex

upload: all flash

clean:
	rm -rf $(TARGET_DIR)

monitor:
	screen $(MONITOR_PORT) 115200
