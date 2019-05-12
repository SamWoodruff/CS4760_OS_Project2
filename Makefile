all: oss user
oss: oss.c clock.c clock.h
	gcc -o oss clock.c clock.h oss.c
user: user.c
	gcc -o user user.c


clean:
	-rm oss user
	
