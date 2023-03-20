FILE=ipkcpc
CC=gcc
CFLAGS=-std=c99 -Wall -Werror -pedantic
D_FLAGS=-fdiagnostics-color=always -g

LOGIN=xponec01

ARGS_UDP=-h 127.0.0.1 -p 2023 -m udp
ARGS_TCP=-h 127.0.0.1 -p 2023 -m tcp

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

test_inputs: make 
	python3 test_inputs.py 

zip:
	zip ${LOGIN}.zip ./imgs/* Makefile test_inputs.py ${FILE}.c README.md LICENSE CHANGELOG.md ${LOGIN}_repo.png
	
