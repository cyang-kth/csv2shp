build:
	g++ -std=gnu++11 -O3 csv2shp.cpp -o csv2shp -lgdal
install:build
	cp csv2shp ~/bin
clean:
	rm csv2shp