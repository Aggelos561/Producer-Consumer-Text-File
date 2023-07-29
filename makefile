all:
	gcc -o consumer consumer.c -lrt -lpthread
	gcc -o producer producer.c my_functions.c -lrt -lpthread

consumer:
	gcc -o consumer consumer.c -lrt -lpthread

producer:
	gcc -o producer producer.c my_functions.c -lrt -lpthread

run:
	./producer $(File) $(K) $(N) 