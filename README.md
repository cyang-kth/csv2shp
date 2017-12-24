### csv2shp

This is a tool for converting from CSV file to ESRI shapefile.

#### Usage

    csv2shp -i INPUT_FILE -x X_COLUMN_NAME, -y Y_COLUMN_NAME|-geom GEOM_COLUMN [-delimiter ';'] [-t 012... ] -o output_file

- t: column types as 0(int),1(double),2(string)

If `t` is not specified, the data types of columns will be infered as int,double,string automatically. 

#### Functions to be added

Manually specify the data types as -t 012012 for the columns

#### Requirement

- Unix OS
- GDAL






