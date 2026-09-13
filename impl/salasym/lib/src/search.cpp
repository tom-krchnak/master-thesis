#include "salasym/search.hpp"

#include "salasym/search/dfs.hpp"

#include "utility/invariants.hpp"

namespace sala::sym
{

ptr< ISearcher > make_searcher( const SearchConfig &config )
{
    switch ( config.strategy )
    {
        case SearchStrategy::DFS:
            return std::make_unique< DFS_Searcher >();
    }

    UNREACHABLE();
}

} // namespace sala::sym
