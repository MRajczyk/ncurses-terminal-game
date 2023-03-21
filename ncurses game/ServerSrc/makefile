all: server

obj/%.o: %.c server.h
	@mkdir -p obj
	gcc -c -o $@ $<

obj/%.o: %.c beast.h
	gcc -c -o $@ $<

obj/%.o: %.c player_handler.h
	gcc -c -o $@ $<

obj/%.o: %.c utils.h
	gcc -c -o $@ $<

server: $(patsubst %,obj/%,main.o server.o beast.o player_handler.o utils.o)
	gcc -o ../$@ $^ -lncursesw -pthread -lrt -lm

clean:
	rm -rf obj ../server