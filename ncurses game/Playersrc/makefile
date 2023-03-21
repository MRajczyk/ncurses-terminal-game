all: player

obj/%.o: %.c player.h
	@mkdir -p obj
	gcc -c -o $@ $<

obj/%.o: %.c utils.h
	gcc -c -o $@ $<

obj/%.o: %.c connection_man.h
	gcc -c -o $@ $<

player: $(patsubst %,obj/%,main.o player.o utils.o connection_man.o)
	gcc -o ../$@ $^ -lncursesw -pthread -lrt

clean:
	rm -rf obj ../player