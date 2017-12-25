## csv2shp

This is a utility tool for converting a CSV file into an ESRI shapefile. 

The CSV file should contain a **header** with geometry stored in format of XY coordinates or WKT text.

#### Installation

    make install

#### Usage

    csv2shp -i INPUT_FILE  -m xy|wkt [-x X_COLUMN_NAME, -y Y_COLUMN_NAME|-geom GEOM_COLUMN] [-delimiter ';'] -o output_file

- i (required): input file
- o (required): output file
- m (required): mode (xy input or wkt input)
- g (optional): geometry column idx (Default:'geom') used when mode is wkt 
- x (optional): x column idx (Default:'x') used when mode is xy 
- y (optional): y column idx (Default:'y') used when mode is xy 
- d (optional): delimiter of CSV file (Default: ';')
- h: help information

The data type of columns will be infered as int,double,string automatically. 

    cd example
    csv2shp -i xy.csv -o example.shp -m 'xy' -d ','
    csv2shp -i wkt.csv -o example.shp -m 'wkt' -d ';'

Check the result using `ogrinfo`:

    ogrinfo -so -al example.shp

#### Requirement

- Unix OS (tested on Ubuntu 16.04)
- gcc >= 4.4 (gnu++11 used)
- GDAL >= 2.1.0

#### Author information 

Can Yang, Ph.D. student at KTH, Royal Institute of Technology in Sweden 

Email: cyang(at)kth.se

Homepage: https://people.kth.se/~cyang/





