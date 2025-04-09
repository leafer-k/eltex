all: main libs

main: main.c
	gcc -o program main.c -ldl

libs: $(patsubst %.c,%.so,$(wildcard libs/*.c))

libs/%.o: libs/%.c
	gcc -fPIC -c $< -o $@

libs/%.so: libs/%.o
	gcc -shared $< -o $@

clean:
	rm -f libs/*.o libs/*.so program
