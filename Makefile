build:initialize
	g++ -std=gnu++11 -O3 csv2shp.cpp -o bin/csv2shp -lgdal
initialize:
	mkdir -p bin
install:build
	cp bin/csv2shp ~/bin
clean:
	rm csv2shp