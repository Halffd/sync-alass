g++ -o bin/sync main.cpp $(pkg-config --cflags --libs gtk+-3.0) -I/usr/local/include/nlohmann/json
cd ./bin/
./sync
