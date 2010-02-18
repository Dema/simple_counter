PRG            = GCC-RTOS
OBJ            = GCC-RTOS.o EERTOS.o EERTOSHAL.o HAL.o
PROGRAMMER     = usbasp
PORT           = usb
MCU_TARGET     = attiny2313
AVRDUDE_TARGET = attiny2313
Q_OPT		   =  -fgcse-las -fgcse-sm -fmodulo-sched -fmodulo-sched-allow-regmoves \
				 -freorder-blocks -freorder-blocks-and-partition -freschedule-modulo-scheduled-loops \
				 -frtl-abstract-sequences -fschedule-insns -fsched-spec-load-dangerous -fsched-stalled-insns=0 
OPTIMIZE       = -Os -fshort-enums -fcse-skip-blocks \
				 -fgcse-after-reload -fipa-cp -fipa-matrix-reorg -fpredictive-commoning 
OPTIMIZE	   += $(Q_OPT)
				 
DEFS           = -I.
LIBS           = 
 
HZ          = 1000000
 
# You should not have to change anything below here.
 
CC             = avr-gcc
 
# Override is only needed by avr-lib build system.
 
override CFLAGS        = -g -DF_CPU=$(HZ)L -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
override LDFLAGS       = -Wl,-Map,$(PRG).map
 
OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump
 
all: $(PRG).elf lst text size #eeprom 
 
$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

size:
		avr-size -C --mcu=$(MCU_TARGET) $(PRG).elf

clean:
	rm -rf *.o $(PRG).elf *.eps *.png *.pdf *.bak *.hex *.bin *.srec
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)
 
lst:  $(PRG).lst
 
%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@
 
# Rules for building the .text rom images
 
text: hex bin srec
 
hex:  $(PRG).hex
bin:  $(PRG).bin
srec: $(PRG).srec
 
%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@
 
%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@
 
%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@
 
# Rules for building the .eeprom rom images
 
eeprom: ehex ebin esrec
 
 
ehex:  $(PRG)_eeprom.hex
#ebin:  $(PRG)_eeprom.bin
esrec: $(PRG)_eeprom.srec
 
%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@
 
#%_eeprom.srec: %.elf
#	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@
 
%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@
 
 
# command to program chip (invoked by running "make install")
install:  $(PRG).hex
	avrdude -p $(AVRDUDE_TARGET) -c $(PROGRAMMER) -P $(PORT) -v  \
         -U flash:w:$(PRG).hex 
 
fuse:
	avrdude -p $(AVRDUDE_TARGET) -c $(PROGRAMMER) -P $(PORT) -v \
	-U lfuse:w:0xc6:m -U hfuse:w:0xd9:m 	
 
ddd: gdbinit
	ddd --debugger "avr-gdb -x $(GDBINITFILE)"
 
gdbserver: gdbinit
	simulavr --device $(MCU_TARGET) --gdbserver
 
gdbinit: $(GDBINITFILE)
 
$(GDBINITFILE): $(PRG).hex
	@echo "file $(PRG).elf" > $(GDBINITFILE)
 
	@echo "target remote localhost:1212" >> $(GDBINITFILE)
	@echo "load"                         >> $(GDBINITFILE)
	@echo "break main"                   >> $(GDBINITFILE)
	@echo
	@echo "Use 'avr-gdb -x $(GDBINITFILE)'"

