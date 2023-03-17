FILE=ipkcpc
CC=gcc
CFLAGS=-std=c99 -Wall -Werror -pedantic
D_FLAGS=-fdiagnostics-color=always -g
ARGS=-h 127.0.0.1 -p 2023 -m udp


ERR_FILE=err.log

make: ${FILE}.c
	${CC} ${CFLAGS} ${FILE}.c -o ${FILE}

debug:
	${CC} ${CFLAGS} ${D_FLAGS} ${FILE}.c -o ${FILE}

run: make
	./${FILE} ${ARGS} 

clean: 
	rm ${FILE} *.log 

