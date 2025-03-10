#include <cstring>
#include <ctime>
#include <cmath>
#include "../regrank/xgboost_regrank_data.h"
#include "../utils/xgboost_utils.h"

using namespace xgboost;
using namespace xgboost::booster;
using namespace xgboost::regrank;

// header in dataset
struct Header{
    FILE *fi;
    int   tmp_num;
    int   base;
    int   num_feat;
    // whether it's dense format
    bool  is_dense;
	bool  warned;
    
	Header( void ){ this->warned = false; this->is_dense = false; }

	inline void CheckBase( unsigned findex ){
		if( findex >= (unsigned)num_feat && ! warned ) {
			fprintf( stderr, "warning:some feature exceed bound, num_feat=%d\n", num_feat );
			warned = true;
		}
	}
};


inline int norm( std::vector<Header> &vec, int base = 0 ){
    int n = base;
    for( size_t i = 0; i < vec.size(); i ++ ){
        if( vec[i].is_dense ) vec[i].num_feat = 1;
        vec[i].base = n; n += vec[i].num_feat;
    }
    return n;        
}

inline void vclose( std::vector<Header> &vec ){
    for( size_t i = 0; i < vec.size(); i ++ ){
        fclose( vec[i].fi );
    }
}

inline int readnum( std::vector<Header> &vec ){
    int n = 0;
    for( size_t i = 0; i < vec.size(); i ++ ){
        if( !vec[i].is_dense ){
            utils::Assert( fscanf( vec[i].fi, "%d", &vec[i].tmp_num ) == 1, "load num" );
            n += vec[i].tmp_num;
        }else{
            n ++;
        }
    }
    return n;        
}

inline void vskip( std::vector<Header> &vec ){
    for( size_t i = 0; i < vec.size(); i ++ ){
        if( !vec[i].is_dense ){
            utils::Assert( fscanf( vec[i].fi, "%*d%*[^\n]\n" ) >= 0 );
        }else{
            utils::Assert( fscanf( vec[i].fi, "%*f\n" ) >= 0 );
        }
    }
}

class DataLoader: public DMatrix{
public:
    // whether to do node and edge feature renormalization
    int rescale;
    int linelimit;
public:
    FILE *fp, *fwlist, *fgroup, *fweight;
    std::vector<Header> fheader;
    std::vector<FMatrixS::REntry> entry;
    DataLoader( void ){
        rescale = 0; 
        linelimit = -1;
        fp = NULL; fwlist = NULL; fgroup = NULL; fweight = NULL;
    }
private:
    inline void Load( std::vector<unsigned> &findex, std::vector<float> &fvalue, std::vector<Header> &vec ){
        unsigned fidx; float fv;
        for( size_t i = 0; i < vec.size(); i ++ ){
            if( !vec[i].is_dense ) { 
                for( int j = 0; j < vec[i].tmp_num; j ++ ){
                    utils::Assert( fscanf ( vec[i].fi, "%u:%f", &fidx, &fv ) == 2, "Error when load feat" );  
                    vec[i].CheckBase( fidx );
                    fidx += vec[i].base;
                    findex.push_back( fidx ); fvalue.push_back( fv );
                }
            }else{
                utils::Assert( fscanf ( vec[i].fi, "%f", &fv ) == 1, "load feat" );  
                fidx = vec[i].base;
                findex.push_back( fidx ); fvalue.push_back( fv );
            }
        }
    }
    inline void DoRescale( std::vector<float> &vec ){
        double sum = 0.0;
        for( size_t i = 0; i < vec.size(); i ++ ){
            sum += vec[i] * vec[i];
        } 
        sum = sqrt( sum );
        for( size_t i = 0; i < vec.size(); i ++ ){
            vec[i] /= sum;
        } 
    }    
public:    
    // basically we are loading all the data inside
    inline void Load( void ){
        this->data.Clear();
        float label, weight = 0.0f;

        unsigned ngleft = 0, ngacc = 0;
        if( fgroup != NULL ){
            info.group_ptr.clear(); 
            info.group_ptr.push_back(0);
        }

        while( fscanf( fp, "%f", &label ) == 1 ){            
            if( ngleft == 0 && fgroup != NULL ){
                utils::Assert( fscanf( fgroup, "%u", &ngleft ) == 1 );
            }
            if( fweight != NULL ){
                utils::Assert( fscanf( fweight, "%f", &weight ) == 1 );
            }
            
            ngleft -= 1; ngacc += 1;

            int pass = 1;
            if( fwlist != NULL ){
                utils::Assert( fscanf( fwlist, "%u", &pass ) ==1 );
            }
            if( pass == 0 ){
                vskip( fheader ); ngacc -= 1;
            }else{            
                const int nfeat = readnum( fheader );
                std::vector<unsigned> findex;
                std::vector<float> fvalue;
                // pairs 
                this->Load( findex, fvalue, fheader );
                utils::Assert( findex.size() == (unsigned)nfeat );
                if( rescale != 0 ) this->DoRescale( fvalue );
                // push back data :)
                this->info.labels.push_back( label );
                // push back weight if any
                if( fweight != NULL ){
                    this->info.weights.push_back( weight );                    
                }
                this->data.AddRow( findex, fvalue );
            }             
            if( ngleft == 0 && fgroup != NULL && ngacc != 0 ){
                info.group_ptr.push_back( info.group_ptr.back() + ngacc );
                utils::Assert( info.group_ptr.back() == data.NumRow(), "group size must match num rows" );
                ngacc = 0;
            }
            // linelimit
            if( linelimit >= 0 ) {
                if( -- linelimit <= 0 ) break;
            }
        }
        if( ngleft == 0 && fgroup != NULL && ngacc != 0 ){
            info.group_ptr.push_back( info.group_ptr.back() + ngacc );
            utils::Assert( info.group_ptr.back() == data.NumRow(), "group size must match num rows" );
        }
        this->data.InitData();
    }
};

const char *folder = "features";

int main( int argc, char *argv[] ){
    if( argc < 3 ){
        printf("Usage:xgcombine_buffer <inname> <outname> [options] -f [features] -fd [densefeatures]\n"\
               "options: -rescale -linelimit -fgroup <groupfilename> -wlist <whitelistinstance>\n");
        return 0; 
    }

    DataLoader loader;
    time_t start = time( NULL );

    int mode = 0;
    for( int i = 3; i < argc; i ++ ){        
        if( !strcmp( argv[i], "-f") ){
            mode = 0; continue;
        }
        if( !strcmp( argv[i], "-fd") ){
            mode = 2; continue;
        }
        if( !strcmp( argv[i], "-rescale") ){
            loader.rescale = 1; continue;
        }
        if( !strcmp( argv[i], "-wlist") ){
            loader.fwlist = utils::FopenCheck( argv[ ++i ], "r" ); continue;
        }
        if( !strcmp( argv[i], "-fgroup") ){
            loader.fgroup = utils::FopenCheck( argv[ ++i ], "r" ); continue;
        }
        if( !strcmp( argv[i], "-fweight") ){
            loader.fweight = utils::FopenCheck( argv[ ++i ], "r" ); continue;
        }
        if( !strcmp( argv[i], "-linelimit") ){
            loader.linelimit = atoi( argv[ ++i ] ); continue;
        }
       
        char name[ 256 ];
        sprintf( name, "%s/%s.%s", folder, argv[1], argv[i] );
        Header h;
        h.fi = utils::FopenCheck( name, "r" );

        if( mode == 2 ){
            h.is_dense = true; h.num_feat = 1;
            loader.fheader.push_back( h );
        }else{
            utils::Assert( fscanf( h.fi, "%d", &h.num_feat ) == 1, "num feat" );
            switch( mode ){
            case 0: loader.fheader.push_back( h ); break;
            default: ;
            }             
        }
    }
    loader.fp = utils::FopenCheck( argv[1], "r" );
    
    printf("num_features=%d\n", norm( loader.fheader ) ); 
    printf("start creating buffer...\n");
    loader.Load();
    loader.SaveBinary( argv[2] );
    // close files
    fclose( loader.fp );
    if( loader.fwlist != NULL ) fclose( loader.fwlist );    
    if( loader.fgroup != NULL ) fclose( loader.fgroup );    
    vclose( loader.fheader );
    printf("all generation end, %lu sec used\n", (unsigned long)(time(NULL) - start) );    
    return 0;
}
