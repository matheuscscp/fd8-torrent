all:
	g++ -std=c++1y ./src/*.cpp -o fd8-torrent -I./3rd/include/SDL2 -L./3rd/lib -lws2_32 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_net
