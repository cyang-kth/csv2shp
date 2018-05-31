## csv2shp

This is a utility tool for converting a CSV file into an ESRI shapefile. 

The CSV file should contain geometry stored in format of XY coordinates or WKT text.

#### Installation

    make install

#### Configurations

    csv2shp -i INPUT_FILE  -m xy|wkt [-x x_name_or_index, -y y_name_or_index |-geom geom_name_or_index ] [-delimiter ';'] -o output_file

- i (required): input file
- o (required): output file
- m (required): mode (xy input or wkt input)
- g (optional): geometry column or idx (Default:'geom') used when mode is wkt 
- x (optional): x column or idx (Default:'x') used when mode is xy 
- y (optional): y column or idx (Default:'y') used when mode is xy 
- d (optional): delimiter of CSV file (Default: ';')
- H (optional): with header, t or f (Default: 't')
- h: help information

The data type of columns will be infered as int,double,string automatically. Without header (`-H f`), the columns will be named as field1, field2, ...

#### Example usage

    cd example
    csv2shp -i xy.csv -o example.shp -m 'xy' -d ','
    csv2shp -i xy.csv -o example.shp -m 'xy' -d ',' -x 3 -y4
    csv2shp -i wkt.csv -o example.shp -m 'wkt' -d ';'
    csv2shp -i wkt.csv -o example.shp -m 'wkt' -d ';' -g 2
    csv2shp -i xy_noheader.csv -o example.shp -m 'xy' -d ',' -x 3 -y 4 -H 'f'
    csv2shp -i wkt_noheader.csv -o example.shp -m 'wkt' -g 2 -H 'f'

Check the result using `ogrinfo`:

    ogrinfo -al example.shp

#### Requirement

- Unix OS (tested on Ubuntu 16.04)
- gcc >= 4.4 (gnu++11 used)
- GDAL >= 2.1.0

#### Author information 

Can Yang, Ph.D. student at KTH, Royal Institute of Technology in Sweden 

Email: cyang(at)kth.se

Homepage: https://people.kth.se/~cyang/





