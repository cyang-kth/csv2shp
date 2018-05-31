/**
 *  Author: Can Yang
 *  Email: cyang (at) kth.se
 *  Date: 2017-12-21
 */
#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <getopt.h>

typedef std::vector<std::string> CSVRow;
std::vector<std::string> GEOM_TYPES = {"POINT", "LINESTRING", "POLYGON", "MULTIPOINT", "MULTILINESTRING", "MULTIPOLYGON"};
struct CSVConfig {
    std::string input_file;
    std::string output_file;
    std::string x_column = "x";
    std::string y_column = "y";
    std::string geom_column = "geom";
    // int x_idx = -1;
    // int y_idx = -1;
    // int geom_idx = -1;
    bool hasHeader = true;
    bool mode_xy = true;
    bool xy = false;
    bool wkt = false;
    char delimiter = ';';
};


class CSVReader {
public:
    CSVReader(CSVConfig &conf): ifs(conf.input_file)
    {
        //std::cout<<"Reading meta data from: " << filename << std::endl;
        delimiter = conf.delimiter;
        if (conf.mode_xy) {
            geom_type_idx = 0; // XY with Point data type
            if (conf.hasHeader) {
                headers = getNextLineAndSplitIntoTokens();
                int num_cols = headers.size();
                std::cout << "Columns count: " << num_cols << std::endl;
                std::vector<std::string> row = getNextLineAndSplitIntoTokens();
                for (int i = 0; i < headers.size(); ++i) {
                    if (headers[i] == conf.x_column) x_idx = i;
                    if (headers[i] == conf.y_column) y_idx = i;
                }
                if (getStringType(conf.x_column)==0) 
                    x_idx = std::stoi(conf.x_column);
                if (getStringType(conf.y_column)==0) 
                    y_idx = std::stoi(conf.y_column);
                // Check if int is specified or not
                if ((x_idx > -1 && x_idx < num_cols) && (y_idx > -1 && y_idx < num_cols)) {
                    std::cout << "X column idx: " << x_idx << std::endl;
                    std::cout << "Y column idx: " << y_idx << std::endl;
                } else {
                    std::cout << "--- Error, x,y column not found" << std::endl;
                    std::cout << "Program stops unexpectedly" << std::endl;
                    exit(EXIT_FAILURE);
                }
                column_types = find_data_types(row);
                // Reset header of the CSV file
                ifs.clear();
                ifs.seekg(0, std::ios::beg);
                getNextLineAndSplitIntoTokens(); // skip header
            } else {
                // Construct header and infer column type
                std::vector<std::string> row = getNextLineAndSplitIntoTokens();
                int num_cols = row.size();
                std::cout << "Columns count: " << num_cols << std::endl;
                headers = get_default_headers(row.size());
                // Check if int is specified or not
                if (getStringType(conf.x_column)==0) 
                    x_idx = std::stoi(conf.x_column);
                if (getStringType(conf.y_column)==0) 
                    y_idx = std::stoi(conf.y_column);
                if (x_idx < 0 || x_idx >= num_cols || y_idx < 0 || y_idx >= num_cols) {
                    std::cout << "--- Error, x,y column not found" << std::endl;
                    std::cout << "Program stops unexpectedly" << std::endl;
                    exit(EXIT_FAILURE);
                }
                column_types = find_data_types(row);
                // Reset header of the CSV file
                ifs.clear();
                ifs.seekg(0, std::ios::beg);
            }
        } else {
            // wkt geom
            if (conf.hasHeader) {
                headers = getNextLineAndSplitIntoTokens();
                int num_cols = headers.size();
                std::cout << "Columns count: " << num_cols << std::endl;
                std::vector<std::string> row = getNextLineAndSplitIntoTokens();
                for (int i = 0; i < headers.size(); ++i) {
                    if (headers[i] == conf.geom_column) geom_idx = i;
                }
                if (getStringType(conf.geom_column)==0) 
                    geom_idx = std::stoi(conf.geom_column);
                // Check if int is specified or not
                if (geom_idx > -1) {
                    std::cout << "Geometry column index " << geom_idx << std::endl;
                } else {
                    std::cout << "--- Error, geometry column not found" << std::endl;
                    std::cout << "Program stops unexpectedly" << std::endl;
                    exit(EXIT_FAILURE);
                }
                std::string wkt = row[geom_idx];
                std::stringstream  wktStream(wkt);
                // Get geometry type from wkt
                // POINT, LINESTRING, POLYGON, MULTILINESTRING, MULTIPOLYGON
                std::string wktType;
                std::getline(wktStream, wktType, ' ');
                for (int i = 0; i < GEOM_TYPES.size(); ++i) {
                    if (wktType == GEOM_TYPES[i]) geom_type_idx = i;
                }
                if (geom_type_idx > -1) {
                    std::cout << "Geometry type " << GEOM_TYPES[geom_type_idx] << std::endl;
                } else {
                    std::cout << "--- Error, unrecognized geometry type: " << wktType << std::endl;
                    std::cout << "Program stops unexpectedly" << std::endl;
                    exit(EXIT_FAILURE);
                }
                column_types = find_data_types(row);
                // Reset header of the CSV file
                ifs.clear();
                ifs.seekg(0, std::ios::beg);
                getNextLineAndSplitIntoTokens(); // skip header
            } else {
                std::vector<std::string> row = getNextLineAndSplitIntoTokens();
                int num_cols = row.size();
                std::cout << "Columns count: " << num_cols << std::endl;
                headers = get_default_headers(row.size());
                if (getStringType(conf.geom_column)==0) 
                    geom_idx = std::stoi(conf.geom_column);
                if (geom_idx > -1 && geom_idx < num_cols) {
                    std::cout << "Geometry column index " << geom_idx << std::endl;
                } else {
                    std::cout << "--- Error, geometry column not found" << std::endl;
                    std::cout << "Program stops unexpectedly" << std::endl;
                    exit(EXIT_FAILURE);
                }
                std::string wkt = row[geom_idx];
                std::stringstream  wktStream(wkt);
                // Get geometry type from wkt
                // POINT, LINESTRING, POLYGON, MULTILINESTRING, MULTIPOLYGON
                std::string wktType;
                std::getline(wktStream, wktType, ' ');
                for (int i = 0; i < GEOM_TYPES.size(); ++i) {
                    if (wktType == GEOM_TYPES[i]) geom_type_idx = i;
                }
                if (geom_type_idx > -1) {
                    std::cout << "Geometry type " << GEOM_TYPES[geom_type_idx] << std::endl;
                } else {
                    std::cout << "--- Error, unrecognized geometry type: " << wktType << std::endl;
                    std::cout << "Program stops unexpectedly" << std::endl;
                    exit(EXIT_FAILURE);
                }
                column_types = find_data_types(row);
                // Reset header of the CSV file
                ifs.clear();
                ifs.seekg(0, std::ios::beg);
            }
        }
        std::cout << "Finish reading meta data" << std::endl;
    };
    std::vector<std::string> getNextLineAndSplitIntoTokens()
    {
        std::vector<std::string>   result;
        std::string                line;
        // extracts characters until the given character is found
        std::getline(ifs, line);
        std::stringstream          lineStream(line);
        std::string                cell;
        while (std::getline(lineStream, cell, delimiter)) // Get token from stringstream into string
        {
            result.push_back(cell);
        }
        return result;
    };
    bool hasNextLine() {
        return ifs.peek() != EOF;
    };
    int get_x_idx() {
        return x_idx;
    }
    int get_y_idx() {
        return y_idx;
    }
    int get_geom_idx() {
        return geom_idx;
    }
    int get_geom_type_idx() {
        return geom_type_idx;
    };
    std::vector<std::string> get_headers() {
        return headers;
    };
    std::vector<int> get_column_types() {
        return column_types;
    };
private:
    static std::vector<int> find_data_types(CSVRow &row) {
        // String 0, int 1, double 2
        std::vector<int> result;
        for (int i = 0; i < row.size(); ++i) {
            result.push_back(getStringType(row[i]));
        };
        return result;
    };
    static std::vector<std::string> get_default_headers(int n) {
        std::vector<std::string> headers(n);
        for (int i = 0; i < n; ++i) {
            headers[i] = "Field" + std::to_string(i);
        }
        return headers;
    };
    /**
     * Get type of string, 0 for int, 1 for double and 2 for string
     * @param  s [description]
     * @return   [description]
     */
    static int getStringType(const std::string & s) {
        if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return 2;
        char* iendptr;
        char* dendptr;
        long ivalue = strtol(s.c_str(), &iendptr, 10);
        double dval = strtod(s.c_str(), &dendptr);
        if (*iendptr != '\0' && *dendptr != '\0') {
            // Input string
            return 2;
        } else if (*iendptr != '\0' && *dendptr == '\0') {
            // Input is double
            return 1;
        } else {
            // Input is integer
            return 0;
        }
    };
    /**
     *  Infer geometry type of the CSV file
     */
    std::vector<std::string> headers; // headers of CSV file
    std::vector<int> column_types; // headers of CSV file
    std::fstream ifs;
    char delimiter = ','; // delimiter
    int x_idx = -1; // x column idx
    int y_idx = -1; // y column idx
    int geom_idx = -1; // geom column idx
    int geom_type_idx = -1; // 0 for point, 1 for linestring and 2 for polygon
};
class SHPWriter {
public:
    // void (*FunctionPointers[3])(OGRFeature *feature, int idx, std::string &content) = {feature_set_int_field,feature_set_double_field,feature_set_string_field};
    SHPWriter(CSVConfig &_conf, CSVReader &reader): conf(_conf) {
        std::string output_file = conf.output_file;
        std::vector<std::string> headers = reader.get_headers();
        x_idx = reader.get_x_idx();
        y_idx = reader.get_y_idx();
        geom_idx = reader.get_geom_idx();
        csv_column_types = reader.get_column_types();
        int geom_type_idx = reader.get_geom_type_idx();
        const char *pszDriverName = "ESRI Shapefile";
        GDALDriver *poDriver;
        GDALAllRegister();
        poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );
        if ( poDriver == NULL )
        {
            printf( "%s driver not available.\n", pszDriverName );
            std::cout << "Program stops unexpectedly" << std::endl;
            exit( 1 );
        }
        poDS = poDriver->Create(output_file.c_str(), 0, 0, 0, GDT_Unknown, NULL );
        if ( poDS == NULL )
        {
            printf( "Creation of output file failed.\n" );
            std::cout << "Program stops unexpectedly" << std::endl;
            exit( 1 );
        }
        poLayer = poDS->CreateLayer("layer", NULL, ogrtypes[geom_type_idx], NULL ); // To update this one
        if ( poLayer == NULL )
        {
            printf( "Layer creation failed.\n" );
            std::cout << "Program stops unexpectedly" << std::endl;
            exit( 1 );
        }
        csv_field_indices = std::vector<int>(headers.size());
        int temp = 0;
        for (int i = 0; i < headers.size(); ++i) {
            // Define fields
            if ((!conf.mode_xy && i == geom_idx) || (conf.mode_xy && i == x_idx)
                    || (conf.mode_xy && i == y_idx)) {
                csv_field_indices[i] = -1;
                continue;
            }
            if (csv_column_types[i] == 0) {
                // int
                OGRFieldDefn oField(headers[i].c_str(), OFTInteger);
                csv_field_indices[i] = temp;
                temp++;
                if (poLayer->CreateField( &oField ) != OGRERR_NONE )
                {
                    printf("Creating int field %s failed.\n", headers[i].c_str());
                    std::cout << "Program stops unexpectedly" << std::endl;
                    exit( 1 );
                }
            } else if (csv_column_types[i] == 1) {
                // Double
                OGRFieldDefn oField(headers[i].c_str(), OFTReal);
                csv_field_indices[i] = temp;
                temp++;
                if ( poLayer->CreateField( &oField ) != OGRERR_NONE )
                {
                    printf("Creating double field %s failed.\n", headers[i].c_str());
                    std::cout << "Program stops unexpectedly" << std::endl;
                    exit( 1 );
                }
            } else { // string
                OGRFieldDefn oField(headers[i].c_str(), OFTString);
                csv_field_indices[i] = temp;
                temp++;
                oField.SetWidth(100);
                if ( poLayer->CreateField( &oField ) != OGRERR_NONE )
                {
                    printf("Creating string field %s failed.\n", headers[i].c_str());
                    std::cout << "Program stops unexpectedly" << std::endl;
                    exit( 1 );
                }
            }
        }
        printf("Fields count excluding geometry %d.\n", temp);
    };
    void write_data(CSVRow &row) {
        if (conf.mode_xy) {
            // XY format
            OGRFeature *poFeature;
            poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
            double x, y;
            for (int i = 0; i < row.size(); ++i) {
                if (i != x_idx && i != y_idx)
                    funcArray[csv_column_types[i]](poFeature, csv_field_indices[i], row[i]);
                // funcArray[column_types[i]](poFeature, headers[i], row[i]);
                if (i == x_idx) x = std::stod(row[i]);
                if (i == y_idx) y = std::stod(row[i]);
            }
            OGRPoint pt;
            pt.setX( x );
            pt.setY( y );
            // When the geometry is created from OGRGeometryFactory, the SetGeometryDirectly should be used.
            poFeature->SetGeometry(&pt);
            if ( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
            {
                printf( "Failed to create feature in shapefile.\n");
                std::cout << "Program stops unexpectedly" << std::endl;
                exit( 1 );
            }
            OGRFeature::DestroyFeature(poFeature);
        } else {
            // WKT format
            OGRFeature *poFeature;
            poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
            for (int i = 0; i < row.size(); ++i) {
                if (i != geom_idx)
                    funcArray[csv_column_types[i]](poFeature, csv_field_indices[i], row[i]);
            }
            OGRGeometry *poGeometry;
            char* pszWKT = const_cast<char*>(row[geom_idx].c_str());
            OGRGeometryFactory::createFromWkt(&pszWKT, NULL, &poGeometry);
            poFeature->SetGeometryDirectly(poGeometry);
            if ( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
            {
                printf( "Failed to create feature in shapefile.\n");
                std::cout << "Program stops unexpectedly" << std::endl;
                exit( 1 );
            }
            OGRFeature::DestroyFeature( poFeature );
        };
    };
    void close() {
        GDALClose(poDS);
    };
    static void feature_set_int_field(OGRFeature *feature, int field_idx, std::string &content) {
        int value = std::stoi(content);
        feature->SetField(field_idx, value);
    };
    static void feature_set_double_field(OGRFeature *feature, int field_idx, std::string &content) {
        double value = std::stod(content);
        feature->SetField(field_idx, value);
    };
    static void feature_set_string_field(OGRFeature *feature, int field_idx, std::string &content) {
        feature->SetField(field_idx, content.c_str());
    };
    // The size of array in class member should be explicitly specified if defined as C array.
    // fp funcArray[3]={...};
    typedef void (*fp)(OGRFeature *feature, int field_idx, std::string &content);
    std::vector<fp> funcArray = {feature_set_int_field, feature_set_double_field, feature_set_string_field};
    std::vector<OGRwkbGeometryType> ogrtypes = {wkbPoint, wkbLineString, wkbPolygon, wkbMultiPoint, wkbMultiLineString, wkbMultiPolygon};
    GDALDataset *poDS;
    OGRLayer *poLayer;
    CSVConfig &conf;
    int x_idx = -1;
    int y_idx = -1;
    int geom_idx = -1;
    std::vector<int> csv_field_indices;
    std::vector<int> csv_column_types;
}; // SHPWriter

void print_help()
{
    std::cout << "Usage: csv2shp -i INPUT_FILE -m XY|WKT (-x X_COLUMN_NAME, -y Y_COLUMN_NAME|-g GEOM_COLUMN) [-d ';'] -o output_file" << std::endl;
    std::cout << "Arguments: " << std::endl;
    std::cout << "-i (required): input file" << std::endl;
    std::cout << "-o (required): output file" << std::endl;
    std::cout << "-m (required): mode (xy or wkt)" << std::endl;
    std::cout << "-g (optional): geometry column idx (Default:'geom')" << std::endl;
    std::cout << "-x (optional): x column idx (Default:'x')" << std::endl;
    std::cout << "-y (optional): y column idx (Default:'y')" << std::endl;
    std::cout << "-d (optional): delimiter of CSV file (Default: ';')" << std::endl;
    std::cout << "-H (optional): with header (t or f) (Default: 't')" << std::endl;
    std::cout << "-h: help information" << std::endl;
};

/**
 * Check if a file exists or not
 * @param  filename
 * @return
 */
bool fileExists(std::string &filename)
{
    const char *filename_c_str = filename.c_str();
    struct stat buf;
    if (stat(filename_c_str, &buf) != -1)
    {
        return true;
    }
    return false;
};

void print_config(CSVConfig &conf){
    std::cout << "Config information" << std::endl;
    std::cout << "    Input file: " << conf.input_file << std::endl;
    std::cout << "    Output file: " << conf.output_file << std::endl;
    std::cout << "    Mode: " << (conf.mode_xy?"xy":"wkt") << std::endl;
    if (conf.mode_xy) {
        std::cout << "    x column or idx:" << conf.x_column << std::endl;
        std::cout << "    y column or idx:" << conf.y_column << std::endl;
    } else {
        std::cout << "    wkt geom column or idx: " << conf.geom_column << std::endl;
    }
    std::cout << "    Delimiter: " << conf.delimiter << std::endl;
    std::cout << "    With header: " << (conf.hasHeader?"t":"f") << std::endl;
};

int main (int argc, char **argv)
{
    std::cout << "--------- csv2shp ---------" << std::endl;
    std::cout << "---- Author: Can Yang  ----" << std::endl;
    CSVConfig conf;
    // std::string input_file;
    // std::string output_file;
    // std::string x_column = "x";
    // std::string y_column = "y";
    // std::string geom_column = "geom";
    // char delimiter = ';';
    std::string mode;
    std::string header_flag = "t";
    // bool xy = false;
    // bool wkt = false;
    // 0 or all patterns, 1 for closed pattern, 2 maximal pattern
    int opt;
    // The last element of the array has to be filled with zeros.
    static struct option long_options[] =
    {
        {"input",  required_argument, 0, 'i' },
        {"output", required_argument, 0, 'o' },
        {"mode", required_argument, 0, 'm' },
        {"help",   no_argument, 0, 'h' },
        {"delimiter",   required_argument, 0, 'd' },
        {"xcol",   required_argument, 0, 'x' },
        {"ycol",   required_argument, 0, 'y' },
        {"geom",   required_argument, 0, 'g' },
        {"header",   required_argument, 0, 'H' },
        {0,         0,                 0,  0 }
    };
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "i:o:m:x:y:g:d:H:h",
                              long_options, &long_index )) != -1)
    {
        switch (opt)
        {
        case 'i' :
            conf.input_file = std::string(optarg);
            break;
        case 'o' :
            conf.output_file = std::string(optarg);
            break;
        case 'd' :
            conf.delimiter = *optarg;
            break;
        case 'm':
            mode = std::string(optarg);
            break;
        case 'x' :
            conf.x_column = std::string(optarg);
            break;
        case 'y' :
            conf.y_column = std::string(optarg);
            break;
        case 'g' :
            conf.geom_column = std::string(optarg);
            break;
        case 'H' :
            header_flag = std::string(optarg);
            break;
        case 'h' :
            print_help();
            exit(EXIT_SUCCESS);
        default:
            print_help();
            exit(EXIT_FAILURE);
        }
    }
    if (!fileExists(conf.input_file))
    {
        std::cout << "--- Error: Input file not found" << std::endl;
        std::cout << "" << conf.input_file << std::endl;
        std::cout << "Program stops unexpectedly" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (mode == "xy") {
        conf.mode_xy = true;
    } else if (mode == "wkt") {
        conf.mode_xy = false;
    } else {
        std::cout << "--- Error -m should be either xy or wkt" << std::endl;
        std::cout << "Program stops unexpectedly" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (header_flag == "t") {
        conf.hasHeader = true;
    } else if (header_flag == "f") {
        conf.hasHeader = false;
    } else {
        std::cout << "--- Error -H should be either t or f" << std::endl;
        std::cout << "Program stops unexpectedly" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Read configuration finished" << std::endl;
    print_config(conf);
    CSVReader reader(conf);
    std::vector<int> column_types = reader.get_column_types();
    std::vector<std::string> headers = reader.get_headers();

    int geom_type_idx = reader.get_geom_type_idx();
    SHPWriter writer(conf, reader);
    int features = 0;
    std::cout << "Write to file " << conf.output_file << std::endl;
    while (reader.hasNextLine()) {
        CSVRow row = reader.getNextLineAndSplitIntoTokens();
        writer.write_data(row);
        ++features;
    }
    std::cout << "Totally " << features << " features created" << std::endl;
    std::cout << "---------------------------" << std::endl;
}
