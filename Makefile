run:
	gcc src/main.c -lraylib -lm -g -Wall -Wextra -Wpedantic -Werror -O2 -o rut && ./rut

experimental:
	gcc src/experimental.c -lraylib -lm -g -O0 -o ex_rut && ./ex_rut
