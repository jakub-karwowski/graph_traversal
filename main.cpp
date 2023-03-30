#include <cstring>
#include <fstream>
#include <iostream>

#include "my_graphs.h"

template <typename this_type>
void dfs_print(const graph_adj_list<this_type>& graph, this_type start, bool build_tree = false) {
    pre_visit_order<this_type> write_out(std::cout);
    empty_fun_obj<this_type> empty_post;
    if (build_tree) {
        tree_build<this_type> tree_build(graph.no_vertices());
        graph_dfs<this_type>(graph, start, write_out, empty_post, tree_build);
        std::cout << std::endl;
        for (auto e : tree_build.get_tree()) {
            std::cout << e << " ";
        }
        std::cout << std::endl;
    } else {
        graph_dfs<this_type>(graph, start, write_out, empty_post, empty_post);
    }
}

template <typename this_type>
void bfs_print(const graph_adj_list<this_type>& graph, this_type start, bool build_tree = false) {
    pre_visit_order<this_type> write_out(std::cout);
    if (build_tree) {
        tree_build<this_type> tree_build(graph.no_vertices());
        graph_bfs<this_type>(graph, start, tree_build, write_out);
        std::cout << std::endl;
        for (auto e : tree_build.get_tree()) {
            std::cout << e << " ";
        }
        std::cout << std::endl;
    } else {
        empty_fun_obj<this_type> empty_post;
        graph_bfs<this_type>(graph, start, empty_post, write_out);
        std::cout << std::endl;
    }
}

template <typename this_type>
void toposort_print(const graph_adj_list<this_type>& graph, bool print_list) {
    if (auto list = graph_topological_sort<this_type>(graph)) {
        std::cout << "graf acykliczny\n";
        if (print_list) {
            for (auto a : *list) {
                std::cout << a << " ";
            }
        }
        std::cout << std::endl;
    } else {
        std::cout << "graf zawiera cykl\n";
    }
}

template <typename T>
void components_print(const graph_adj_list<T>& graph, bool print_components) {
    auto components = graph_strongly_connected(graph);
    std::cout << "#składowych: " << components.size() << "\n";
    std::cout << "#wierchołków w składowych: ";
    for (auto& c : components) {
        std::cout << c.size() << " ";
    }
    std::cout << std::endl;
    if (print_components) {
        for (auto& c : components) {
            std::cout << "{";
            for (auto e : c) {
                std::cout << e << ", ";
            }
            std::cout << "}\n";
        }
    }
}

template <typename T>
void bipart_print(const graph_adj_list<T> graph, bool print_subsets) {
    if (auto list = graph_bipart(graph)) {
        std::cout << "graf dwudzielny\n";
        if (print_subsets) {
            for (auto a : *list) {
                std::cout << static_cast<int>(a) << " ";
            }
            std::cout << std::endl;
        }
    } else {
        std::cout << "graf nie jest dwudzielny\n";
    }
}

int main(int argc, char** argv) {
    using this_type = unsigned;
    enum mode {
        dfs = 0,
        bfs,
        sort,
        connected,
        bipartite,
        no_modes,
    };
    static constexpr const char* mode_names[mode::no_modes] = {
        "dfs",
        "bfs",
        "sort",
        "connected",
        "bipartite",
    };
    if (argc < 2) {
        std::cout << "za mało argumentów\n";
        return 1;
    }
    int option = mode::no_modes;
    const char* file_name = argv[1];
    for (int i = 0; i < mode::no_modes; ++i) {
        if (std::strcmp(mode_names[i], argv[2]) == 0) {
            option = i;
            break;
        }
    }

    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cout << "nie znaleziono pliku\n";
        return 1;
    }
    graph_adj_list<this_type> graph = graph_read<this_type>(file);
    file.close();
    switch (option) {
    case mode::dfs:
        std::cout << "** dfs:\n";
        dfs_print<this_type>(graph, 1, true);
        break;
    case mode::bfs:
        std::cout << "** bfs:\n";
        bfs_print<this_type>(graph, 1, true);
        break;
    case mode::sort:
        std::cout << "** sortowanie topologiczne:\n";
        toposort_print(graph, graph.no_vertices() <= 200);
        break;
    case mode::connected:
        std::cout << "** silnie spójne składowe:\n";
        components_print(graph, graph.no_vertices() <= 200);
        break;
    case mode::bipartite:
        std::cout << "** czy graf dwudzielny?:\n";
        bipart_print(graph, graph.no_vertices() <= 200);
        break;
    default:
        std::cout << "nieprawidłowy argument\n";
        return 1;
    }
    return 0;
}
