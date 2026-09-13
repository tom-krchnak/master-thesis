#pragma once

namespace sala::sym
{

enum class SearchStrategy
{
    DFS,
    BFS,
    AStar,
};

struct SearchConfig
{
    SearchStrategy strategy = SearchStrategy::DFS;
};

struct ExecConfig
{
    SearchConfig search;
};

} // namespace sala::sym
