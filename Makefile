Compression: Compression.cpp
	g++ -o $@ $^ -Wall -O2
clean:
	rm *.o *.tmp Compression -f
