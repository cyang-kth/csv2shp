build:
	g++ -std=gnu++11 csv2shp.cpp -o csv2shp -lgdal
test:
	g++ -std=gnu++11 checkstring.cpp -o checkstring.out
install:build
	cp csv2shp ~/bin
clean:
	rm *.out