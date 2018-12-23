RAPIDJSON 	:=	src/libs/rapidjson/include
MINIZIP		:=	src/libs/minizip
MINIZIP_O :=  zip.o ioapi.o unzip.o

build:
	gcc -c $(MINIZIP)/*.c
	g++ -g cli/*.cpp src/*.cpp -std=gnu++11 -lm -I $(RAPIDJSON) $(MINIZIP_O) -I $(MINIZIP) -lz -lcurl -o get

run_tests:
	rm -rf tests/.get/packages tests/.get/tmp
	gcc -c $(MINIZIP)/*.c
	g++ -g tests/*.cpp src/*.cpp -std=gnu++11 -lm -I $(RAPIDJSON) $(MINIZIP_O) -I $(MINIZIP) -lz -lcurl -o get_tests
	cd tests/server && python3 -m http.server &
	export SERVERD=$!
	sleep 2
	./get_tests

clean:
	rm *.o get get_tests
