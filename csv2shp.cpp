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
std::vector<std::string> GEOM_TYPES={"POINT","LINESTRING","POLYGON","MULTIPOINT","MULTILINESTRING","MULTIPOLYGON"};

class CSVReader{
public:
    CSVReader(const std::string & filename, char delim, const std::string & x_column,const std::string & y_column):ifs(filename)
    {
        std::cout<<"Reading meta data from: " << filename << std::endl;
        delimiter = delim;
        // Infer geometry type
        headers = getNextLineAndSplitIntoTokens();
        // Read a single line to infer the geometry type
        std::vector<std::string> row = getNextLineAndSplitIntoTokens();
        for (int i=0;i<headers.size();++i){
            if(headers[i]==x_column) x_idx = i;
            if(headers[i]==y_column) y_idx = i;
        }
        if (x_idx>-1 && y_idx>-1){
            std::cout<<"X column idx: " << x_idx << std::endl;
            std::cout<<"Y column idx: " << y_idx << std::endl;
        } else {
            std::cout<<"Error, x,y column not found" << std::endl;
            exit(EXIT_FAILURE);
        }
        geom_type_idx=0; // XY with Point data type
        column_types = find_data_types(row);
        // Reset header of the CSV file
        ifs.clear();
        ifs.seekg(0, std::ios::beg);
        getNextLineAndSplitIntoTokens(); // skip header
        std::cout<<"Finish reading meta data" << std::endl;
    };
    CSVReader(const std::string & filename,char delim, const std::string & geom_column):ifs(filename)
    {
        std::cout<<"Reading meta data from: " << filename << std::endl;
        delimiter = delim;
        // Infer geometry type
        headers = getNextLineAndSplitIntoTokens();
        // Read a single line to infer the geometry type
        std::vector<std::string> row = getNextLineAndSplitIntoTokens();
        for (int i=0;i<headers.size();++i){
            if(headers[i]==geom_column) geom_idx = i;
        }
        if (geom_idx>-1){
            std::cout<<"Geometry column index "<< geom_idx << std::endl;
        } else {
            std::cout<<"Error, geometry column not found" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::string wkt = row[geom_idx];
        std::stringstream  wktStream(wkt);
        // Get geometry type from wkt
        // POINT, LINESTRING, POLYGON, MULTILINESTRING, MULTIPOLYGON
        std::string wktType;
        std::getline(wktStream,wktType,' ');
        for (int i=0;i<GEOM_TYPES.size();++i){
            if(wktType==GEOM_TYPES[i]) geom_type_idx = i;
        }
        if (geom_type_idx>-1){
            std::cout<<"Geometry type " << GEOM_TYPES[geom_type_idx] << std::endl;
        } else {
            std::cout<<"Error, unrecognized geometry type: " <<wktType<< std::endl;
            exit(EXIT_FAILURE);
        }
        column_types = find_data_types(row);
        // Reset header of the CSV file
        ifs.clear();
        ifs.seekg(0, std::ios::beg);
        getNextLineAndSplitIntoTokens(); // skip header
        std::cout<<"Finish reading meta data" << std::endl;
    };
    std::vector<std::string> getNextLineAndSplitIntoTokens()
    {
        std::vector<std::string>   result;
        std::string                line;
        // extracts characters until the given character is found
        std::getline(ifs,line);
        std::stringstream          lineStream(line);
        std::string                cell;
        while(std::getline(lineStream,cell,delimiter)) // Get token from stringstream into string
        {
            result.push_back(cell);
        }
        return result;
    };
    bool hasNextLine(){
        return ifs.peek()!=EOF;
    };
    int get_x_idx(){
        return x_idx;
    }
    int get_y_idx(){
        return y_idx;
    }
    int get_geom_idx(){
        return geom_idx;
    }
    int get_geom_type_idx(){
        return geom_type_idx;
    };
    std::vector<std::string> get_headers(){
        return headers;
    };
    std::vector<int> get_column_types(){
        return column_types;
    };
private:
    static std::vector<int> find_data_types(CSVRow &row){
        // String 0, int 1, double 2
        std::vector<int> result;
        for (int i=0;i<row.size();++i){
            result.push_back(getStringType(row[i]));
        };
        return result;
    };
    /**
     * Get type of string, 0 for int, 1 for double and 2 for string
     * @param  s [description]
     * @return   [description]
     */
    static int getStringType(const std::string & s){
        if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return 2;
        char* iendptr;
        char* dendptr;
        long ivalue = strtol(s.c_str(), &iendptr, 10);
        double dval = strtod(s.c_str(), &dendptr);
        if (*iendptr != '\0' && *dendptr != '\0'){
          // Input string 
          return 2;
        } else if (*iendptr != '\0' && *dendptr == '\0'){
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
    char delimiter=','; // delimiter
    int x_idx=-1; // x column idx
    int y_idx=-1; // y column idx
    int geom_idx=-1; // geom column idx
    int geom_type_idx=-1; // 0 for point, 1 for linestring and 2 for polygon
};
class SHPWriter{
public:
    static void feature_set_int_field(OGRFeature *feature, std::string &field, std::string &content){
        int value = std::stoi(content);
        feature->SetField(field.c_str(),value);
    };
    static void feature_set_double_field(OGRFeature *feature, std::string &field, std::string &content){
        double value = std::stod(content);
        feature->SetField(field.c_str(),value);
    };
    static void feature_set_string_field(OGRFeature *feature, std::string &field, std::string &content){
        feature->SetField(field.c_str(),content.c_str());
    };
    // The size of array in class member should be explicitly specified if defined as C array. 
    // fp funcArray[3]={...};
    typedef void (*fp)(OGRFeature *feature, std::string &field, std::string &content);
    std::vector<fp> funcArray ={feature_set_int_field,feature_set_double_field,feature_set_string_field};
    std::vector<OGRwkbGeometryType> ogrtypes={wkbPoint,wkbLineString,wkbPolygon,wkbMultiPoint,wkbMultiLineString,wkbMultiPolygon};
    // void (*FunctionPointers[3])(OGRFeature *feature, int idx, std::string &content) = {feature_set_int_field,feature_set_double_field,feature_set_string_field};
    SHPWriter(const std::string &filename,std::vector<std::string> &headers,std::vector<int> &column_types, int geom_type_idx){
        const char *pszDriverName = "ESRI Shapefile";
        GDALDriver *poDriver;
        GDALAllRegister();
        poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );
        if( poDriver == NULL )
        {
            printf( "%s driver not available.\n", pszDriverName );
            exit( 1 );
        }
        poDS = poDriver->Create(filename.c_str(), 0, 0, 0, GDT_Unknown, NULL );
        if( poDS == NULL )
        {
            printf( "Creation of output file failed.\n" );
            exit( 1 );
        }
        poLayer = poDS->CreateLayer("layer", NULL,ogrtypes[geom_type_idx], NULL ); // To update this one
        if( poLayer == NULL )
        {
            printf( "Layer creation failed.\n" );
            exit( 1 );
        }
        for (int i=0;i<headers.size();++i){
            // Define fields
            if(column_types[i]==0){
                // int
                OGRFieldDefn oField(headers[i].c_str(),OFTInteger);
                if( poLayer->CreateField( &oField ) != OGRERR_NONE )
                {
                    printf("Creating int field %s failed.\n", headers[i].c_str());
                    exit( 1 );
                }
            } else if (column_types[i]==1){
                // Double
                OGRFieldDefn oField(headers[i].c_str(),OFTReal);
                if( poLayer->CreateField( &oField ) != OGRERR_NONE )
                {
                    printf("Creating double field %s failed.\n", headers[i].c_str());
                    exit( 1 );
                }
            } else { // string 
                OGRFieldDefn oField(headers[i].c_str(),OFTString);
                oField.SetWidth(100);
                if( poLayer->CreateField( &oField ) != OGRERR_NONE )
                {
                    printf("Creating string field %s failed.\n", headers[i].c_str());
                    exit( 1 );
                }
            }
        }
    };
    void write_data_xy(CSVRow &row,std::vector<std::string> &headers,std::vector<int> &column_types,int x_idx,int y_idx){
        OGRFeature *poFeature;
        poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
        double x, y;
        for (int i=0;i<row.size();++i){
            funcArray[column_types[i]](poFeature,headers[i],row[i]);
            if (i==x_idx) x = std::stod(row[i]);
            if (i==y_idx) y = std::stod(row[i]);
        }
        OGRPoint pt;
        pt.setX( x );
        pt.setY( y );
        // When the geometry is created from OGRGeometryFactory, the SetGeometryDirectly should be used.
        poFeature->SetGeometry(&pt);
        if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in shapefile.\n");
            exit( 1 );
        } 
        OGRFeature::DestroyFeature(poFeature);
    };
    void write_data_wkt(CSVRow &row,std::vector<std::string> &headers,std::vector<int> &column_types,int geom_idx){
        OGRFeature *poFeature;
        poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
        for (int i=0;i<row.size();++i){
            if (i!=geom_idx) funcArray[column_types[i]](poFeature,headers[i],row[i]);
        }
        OGRGeometry *poGeometry;
        char* pszWKT = const_cast<char*>(row[geom_idx].c_str());
        OGRGeometryFactory::createFromWkt(&pszWKT,NULL,&poGeometry);
        poFeature->SetGeometryDirectly(poGeometry);
        if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in shapefile.\n");
            exit( 1 );
        }
        OGRFeature::DestroyFeature( poFeature );
    };
    void close(){
        GDALClose(poDS);
    };
    GDALDataset *poDS;
    OGRLayer *poLayer;
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
    std::cout << "-h: help information" << std::endl;
};

/**
 * Check if a file exists or not
 * @param  filename
 * @return
 */
bool fileExists(std::string &filename)
{
    const char *filename_c_str=filename.c_str();
    struct stat buf;
    if (stat(filename_c_str, &buf) != -1)
    {
        return true;
    }
    return false;
};

int main (int argc, char **argv)
{
    std::cout << "--------- csv2shp ---------" << std::endl;
    std::cout << "---- Author: Can Yang  ----" << std::endl;
    std::string input_file;
    std::string output_file;
    double min_sup_value = 0;
    std::string x_column="x";
    std::string y_column="y";
    std::string geom_column="geom";
    char delimiter=';';
    std::string mode;
    bool xy=false;
    bool wkt=false;
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
        {0,         0,                 0,  0 }
    };
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "i:o:m:d:x:y:g:h",
                              long_options, &long_index )) != -1)
    {
        switch (opt)
        {
            case 'i' :
                input_file = std::string(optarg);
                break;
            case 'o' :
                output_file = std::string(optarg);
                break;
            case 'd' :
                delimiter = *optarg;
                break;
            case 'm':
                mode = std::string(optarg); 
                break;
            case 'x' :
                x_column = std::string(optarg);
                break;
            case 'y' :
                y_column = std::string(optarg);
                break;
            case 'g' :
                geom_column = std::string(optarg);
                break;
            case 'h' :
                print_help();
                exit(EXIT_SUCCESS);
            default:
                print_help();
                exit(EXIT_FAILURE);
        }
    }
    if (!fileExists(input_file))
    {
        std::cout << "Error: Input file not found" << std::endl;
        std::cout << "" << input_file << std::endl;
        print_help();
        exit(EXIT_FAILURE);
    }
    if (mode=="xy"){
        CSVReader reader(input_file, delimiter, x_column,y_column);        
        std::vector<int> column_types = reader.get_column_types();
        std::vector<std::string> headers = reader.get_headers();
        int geom_type_idx = reader.get_geom_type_idx();
        SHPWriter writer(output_file,headers,column_types,geom_type_idx);
        // for (int i=0;i<headers.size();++i){
        //     std::cout << "Column "<<headers[i] <<" type "<< column_types[i] << std::endl;
        // }
        int x_idx = reader.get_x_idx();
        int y_idx = reader.get_y_idx();
        //std::cout <<"Has next line "<<reader.hasNextLine()<< std::endl;
        int features=0;
        while (reader.hasNextLine()){
            CSVRow row = reader.getNextLineAndSplitIntoTokens();
            writer.write_data_xy(row,headers,column_types,x_idx,y_idx);
            ++features;
        }
        std::cout << "Write to file " <<output_file<< std::endl;
        std::cout << "Totally "<< features <<" features created"<< std::endl;
    } else if (mode=="wkt"){
        CSVReader reader(input_file,delimiter,geom_column);
        std::vector<int> column_types = reader.get_column_types();
        std::vector<std::string> headers = reader.get_headers();
        int geom_type_idx = reader.get_geom_type_idx();
        SHPWriter writer(output_file,headers,column_types,geom_type_idx);
        int geom_idx = reader.get_geom_idx();
        int features=0;
        while (reader.hasNextLine()){
            CSVRow row = reader.getNextLineAndSplitIntoTokens();
            writer.write_data_wkt(row,headers,column_types,geom_idx);
            ++features;
        }
        writer.close();
        std::cout << "Write to file " <<output_file<< std::endl;
        std::cout << "Totally "<< features <<" features created"<< std::endl;
    } else {
        std::cout << "Error: unrecognized mode (xy|wkt): "<<mode<< std::endl;
        exit(EXIT_FAILURE);
    };
    std::cout << "---------------------------" << std::endl;
}
