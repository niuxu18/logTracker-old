inline void LoadModel( utils::IStream &fi ){
                utils::Assert( fi.Read( &param, sizeof(Param) ) > 0, "TreeModel" );
                nodes.resize( param.num_nodes ); stats.resize( param.num_nodes );
                utils::Assert( fi.Read( &nodes[0], sizeof(Node) * nodes.size() ) > 0, "TreeModel::Node" );
                utils::Assert( fi.Read( &stats[0], sizeof(NodeStat) * stats.size() ) > 0, "TreeModel::Node" );

                deleted_nodes.resize( 0 );
                for( int i = param.num_roots; i < param.num_nodes; i ++ ){
                    if( nodes[i].is_root() ) deleted_nodes.push_back( i );
                }
                utils::Assert( (int)deleted_nodes.size() == param.num_deleted, "number of deleted nodes do not match" );
            }