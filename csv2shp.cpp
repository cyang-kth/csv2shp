/**
 *  Author: Can Yang
 *  Date: 2017-12-21
 */
#include "ogrsf_frmts.h" // GDAL C++ API
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <sys/stat.h>

typedef std::vector<std::string> CSVRow;
std::vector<std::string> GEOM_TYPES={"POINT","LINESTRING","POLYGON"};
// https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c/4654718#4654718
inline bool isInteger(const std::string & s)
{
    if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;
    char * p ;
    strtol(s.c_str(), &p, 10) ;
    return (*p == 0) ;
};
/**
 * Get type of string, 0 for int, 1 for double and 2 for string
 * @param  s [description]
 * @return   [description]
 */
int getStringType(const std::string & s){
    if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return 2;
    char* p;
    long converted = strtol(s, &p, 10);
    if (*p) {
        // conversion failed because the input wasn't a number
        return 2;
    }
    else {
        // if it is a double

        // if it is an int
    }
};
class CSVReader{
public:
    CSVReader(const std::string & filename, const std::string & x_column,const std::string & y_column)
    {
        std::cout<<"Reading meta data from: " << filename << std::endl;
        // Infer geometry type
        std::fstream ifs(filename);
        headers = getNextLineAndSplitIntoTokens(ifs);
        // Read a single line to infer the geometry type
        std::vector<std::string> row = getNextLineAndSplitIntoTokens(ifs);
        for (int i=0;i<N;++i){
            if(row[i]==x_column) x_idx = i;
            if(row[i]==y_column) y_idx = i;
        }
        if (x_idx>-1 && y_idx>-1){
            std::cout<<"X column idx" << x_idx << std::endl;
            std::cout<<"Y column idx" << y_idx << std::endl;
        } else {
            std::cout<<"Error, x,y column not found" << std::endl;
        }
        geom_type_idx=0; // XY with Point data type
        std::cout<<"Finish reading meta data" << std::endl;
    };
    CSVReader(const std::string & filename, const std::string & geom_column)
    {
        std::cout<<"Reading meta data from: " << filename << std::endl;
        // Infer geometry type
        std::fstream ifs(filename);
        headers = getNextLineAndSplitIntoTokens(ifs);
        // Read a single line to infer the geometry type
        std::vector<std::string> row = getNextLineAndSplitIntoTokens(ifs);
        for (int i=0;i<N;++i){
            if(row[i]==geom_column) geom_idx = i;
        }
        if (geom_idx>-1){
            std::cout<<"Geometry column index" << geom_idx << std::endl;
        } else {
            std::cout<<"Error, geometry column not found" << std::endl;
        }
        std::string wkt = row[geom_idx];
        std::stringstream  wktStream(wkt);
        // Get geometry type from wkt
        // POINT, LINESTRING, POLYGON, MULTILINESTRING, MULTIPOLYGON
        std::string wktType;
        std::getline(wktStream,wktType,'(');
        geom_type_idx = find_geom_type_idx(wktType);
        if (geom_type_idx>-1){
            std::cout<<"Geometry type" << GEOM_TYPES[geom_type_idx] << std::endl;
        } else {
            std::cout<<"Error, unrecognized geometry type" << std::endl;
        }
        std::cout<<"Finish reading meta data" << std::endl;
    };
    std::vector<std::string> getNextLineAndSplitIntoTokens(std::istream& str)
    {
        std::vector<std::string>   result;
        std::string                line;
        // extracts characters until the given character is found
        std::getline(str,line);
        std::stringstream          lineStream(line);
        std::string                cell;
        while(std::getline(lineStream,cell,delimiter)) // Get token from stringstream into string
        {
            result.push_back(cell);
        }
        return result;
    };
    int get_geom_type_idx(){
        return geom_type_idx;
    };
    std::vector<std::string> get_headers(){
        return headers;
    };
private:
    static int find_geom_type_idx(std::string &wkt){
        for (int i=0;i<GEOM_TYPES.size();++i){
            if(wktType==GEOM_TYPES[i]) geom_type_idx = i;
        }
    };
    static std::vector<int> infer_data_types(CSVRow &row){
        // String 0, int 1, double 2
        std::vector<int> result;
        for (int i=0;i<row.size();++i){
            if (std::stod(row[i])){
                result.push_back(2);
            } else if (std::stoi(row[i])){
                result.push_back(1);
            } else {
                result.push_back(0);
            };
        };
        return result;
    };
    /**
     *  Infer geometry type of the CSV file
     */
    std::vector<std::string> headers; // headers of CSV file
    char delimiter=';'; // delimiter
    int x_idx=-1; // x column idx
    int y_idx=-1; // y column idx
    int geom_idx=-1; // geom column idx
    int geom_type_idx=-1; // 0 for point, 1 for linestring and 2 for polygon
};

class SHPWriter{
public:
    typedef void(SetFieldFunc*) (OGRFeature *, int, std::string &);
    void feature_set_int_field(OGRFeature *feature, int idx, std::string &content){
        int value = ;
        feature->SetField(idx,value);
    };
    void feature_set_double_field(OGRFeature *feature, int idx, std::string &content){
        double value = ;
        feature->SetField(idx,value);
    };
    void feature_set_string_field(OGRFeature *feature, int idx, std::string &content){
        feature->SetField(idx,content.c_str());
    };
    SetFieldFunc[] funcArray=[feature_set_int_field,feature_set_double_field,feature_set_string_field];
    SHPWriter(const std::string &filename,std::vector<std::string> &headers,int geom_idx){
        const char *pszDriverName = "ESRI Shapefile";
        GDALDriver *poDriver;
        GDALAllRegister();
        poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );
        if( poDriver == NULL )
        {
            printf( "%s driver not available.\n", pszDriverName );
            exit( 1 );
        }
        poDS = poDriver->Create(filename, 0, 0, 0, GDT_Unknown, NULL );
        if( poDS == NULL )
        {
            printf( "Creation of output file failed.\n" );
            exit( 1 );
        }
        poLayer = poDS->CreateLayer("layer", NULL,wkbPoint, NULL ); // To update this one
        if( poLayer == NULL )
        {
            printf( "Layer creation failed.\n" );
            exit( 1 );
        }
        for (int i=0;i<headers.size();++i){
            OGRFieldDefn oField(headers[i],OFTString);
            oField.SetWidth(32);
            if( poLayer->CreateField( &oField ) != OGRERR_NONE )
            {
                printf("Creating field %s failed.\n", headers[i]);
                exit( 1 );
            }
        }
    };
    void write_data_xy(CSVRow &row,std::vector<int> &fieldtypes){
        OGRFeature *poFeature;
        poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
        double x, y;
        for (int i=0;i<row.size();++i){
            poFeature->SetField(i,row[i]);
            if (i==x_idx) x = row[i];
            if (i==y_idx) y = row[i];
        }
        OGRPoint pt;
        pt.setX( x );
        pt.setY( y );
        poFeature->SetGeometryDirectly(&pt);
        if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in shapefile.\n");
            exit( 1 );
        }
        OGRFeature::DestroyFeature( poFeature );
    };
    void write_data_wkt(CSVRow &row,std::vector<int> &fieldtypes){
        OGRFeature *poFeature;
        poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
        for (int i=0;i<row.size();++i){
            if (i!=geom_idx) poFeature->SetField(i,row[i]);
        }
        OGRGeometry *poGeometry;
        char* pszWKT = row[geom_idx].c_str();
        OGRGeometryFactory::createFromWkt(&pszWKT,NULL,&poGeometry);
        poFeature->SetGeometryDirectly(poGeometry);
        if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
        {
            printf( "Failed to create feature in shapefile at row.\n");
            exit( 1 );
        }
        OGRFeature::DestroyFeature( poFeature );
    };
    void close_writer(){
        GDALClose(poDS);
    };
    GDALDataset *poDS;
    OGRLayer *poLayer;
}; // SHPWriter

void print_help()
{
    std::cout << "--------- csv2shp ---------" << std::endl;
    std::cout << "---- Author: Can Yang  ----" << std::endl;
    std::cout << "Usage: csv2shp -i INPUT_FILE (-x X_COLUMN_NAME, -y Y_COLUMN_NAME|-g GEOM_COLUMN) [-d ';'] -o output_file" << std::endl;
    std::cout << "Arguments: " << std::endl;
    std::cout << "-i (required): input file" << std::endl;
    std::cout << "-o (required): output file" << std::endl;
    std::cout << "-g: geometry column idx" << std::endl;
    std::cout << "-x: x column idx" << std::endl;
    std::cout << "-y: y column idx" << std::endl;
    std::cout << "-d: delimiter of CSV file (Default: ';')" << std::endl;
    std::cout << "-h: with header or not (Default: true)" << std::endl;
    std::cout << "---------------------------" << std::endl;
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
    std::cout << "Read configurations" << std::endl;
    std::string input_file;
    std::string output_file;
    double min_sup_value = 0;
    std::string x_column="x";
    std::string y_column="y";
    std::string geom_column="geom";
    char delimiter=';';
    // 0 or all patterns, 1 for closed pattern, 2 maximal pattern
    int opt;
    // The last element of the array has to be filled with zeros.
    static struct option long_options[] =
    {
        {"input",  required_argument, 0, 'i' },
        {"output", required_argument, 0, 'o' },
        {"help",   no_argument, 0, 'h' },
        {"delimiter",   required_argument, 0, 'd' },
        {"xcol",   required_argument, 0, 'x' },
        {"ycol",   required_argument, 0, 'y' },
        {"geom",   required_argument, 0, 'g' },
        {0,         0,                 0,  0 }
    };
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "i:o:d:x:y:g:h",
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
                delimiter = optarg;
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
        std::cout << "===========================" << std::endl;
        std::cout << "Error: Input file not found:" << std::endl;
        std::cout << "" << input_file << std::endl;
        print_help();
        exit(EXIT_FAILURE);
    }
    CSVReader reader(input_file);
    SHPWriter writer(output_file)

}
