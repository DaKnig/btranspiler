proj_name:=btranspiler
gb_out:=game
bf_in:=input.bf

gb_rom:=$(gb_out).gb

emulator:=wine ~/prog/bgb/bgb.exe


all:	bin/$(gb_rom)
bin/$(gb_rom):	$(gb_out).o included/terminal.o included/my_funcs.o
		rgblink -o bin/$(gb_rom) -n $(gb_out).sym $(gb_out).o included/terminal.o included/my_funcs.o
		rgbfix	-v -m 0 -r 0 -p 0xFF bin/$(gb_rom)
$(gb_out).z80:	$(bf_in) $(proj_name)
		./$(proj_name) $(bf_in) $(gb_out).z80
$(proj_name):	$(proj_name).c
		gcc -o $(proj_name) $(proj_name).c
clean:
		rm *.o $(gb_out).z80 *.tmp $(gb_rom)
%.o: %.z80
		uncomment $< $<.tmp
		rgbasm -o $@ $<.tmp
run:	bin/$(gb_rom)
		$(emulator) $(gb_rom) &

backup: bin/$(gb_rom)
		tar --create -h --file releases/release_$(shell date +%Y.%m.%d).tar $(proj_name) $(proj_name).c included ###
		cp $(proj_name) releases/$(proj_name)_$(shell date +%Y.%m.%d)
