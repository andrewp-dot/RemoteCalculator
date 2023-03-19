FILE=ipkcpc
CC=gcc
CFLAGS=-std=c99 -Wall -Werror -pedantic
D_FLAGS=-fdiagnostics-color=always -g
ARGS_UDP=-h 192.168.1.166 -p 2023 -m udp
ARGS_TCP=-h 192.168.1.166 -p 2023 -m tcp

ERR_FILE=err.log

make: ${FILE}.c
	${CC} ${CFLAGS} ${FILE}.c -o ${FILE}

debug:
	${CC} ${CFLAGS} ${D_FLAGS} ${FILE}.c -o ${FILE}

udp: make
	./${FILE} ${ARGS_UDP} 

tcp: make
	./${FILE} ${ARGS_TCP} 

clean: 
	rm ${FILE} *.log 

win:
	${CC} ${CFLAGS} ${FILE}.c -o ${FILE} -lws2_32
# windows compilation
# gcc -std=c99 -Wall -Werror -pedantic ipkcpc.c -o ipkcpc -lws2_32
# 192.168.1.86:2023