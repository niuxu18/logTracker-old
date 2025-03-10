        class FeatMap{
        public:
            enum Type{
                kIndicator  = 0,
                kQuantitive = 1,
                kInteger = 2,
                kFloat = 3
            };
        public:
            /*! \brief load feature map from text format */
            inline void LoadText( const char *fname ){
                FILE *fi = utils::FopenCheck( fname, "r" );
                this->LoadText( fi );
                fclose( fi );
            }
            /*! \brief load feature map from text format */
            inline void LoadText( FILE *fi ){
                int fid;
                char fname[256], ftype[256];
                while( fscanf( fi, "%d%s%s", &fid, fname, ftype ) == 3 ){
                    utils::Assert( fid == (int)names_.size(), "invalid fmap format" );
                    names_.push_back( std::string(fname) );
                    types_.push_back( GetType( ftype ) );
                }
            }
            /*! \brief number of known features */
            size_t size( void ) const{
                return names_.size();
            }
            /*! \brief return name of specific feature */
            const char* name( size_t idx ) const{
                utils::Assert( idx < names_.size(), "utils::FMap::name feature index exceed bound" );
                return names_[ idx ].c_str();
            }
            /*! \brief return type of specific feature */
            const Type& type( size_t idx ) const{
                utils::Assert( idx < names_.size(), "utils::FMap::name feature index exceed bound" );
                return types_[ idx ];
            }
        private:
            inline static Type GetType( const char *tname ){
                if( !strcmp( "i", tname ) ) return kIndicator;        
                if( !strcmp( "q", tname ) ) return kQuantitive;
                if( !strcmp( "int", tname ) ) return kInteger;        
                if( !strcmp( "float", tname ) ) return kFloat;        
                utils::Error("unknown feature type, use i for indicator and q for quantity");
                return kIndicator;
            }
