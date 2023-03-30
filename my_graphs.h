#ifndef MY_GRAPHS_H
#define MY_GRAPHS_H

#include <exception>
#include <iostream>
#include <optional>
#include <queue>
#include <stack>
#include <vector>

class file_format_exception : public std::exception {
    const char* what() const noexcept override {
        return "wrong file format";
    }
};

enum class graph_type { directed,
                        undirected };

template <typename T>
class graph_adj_list {
private:
    std::vector<std::vector<T>> list_;
    graph_type type_;

public:
    graph_adj_list(graph_type type, std::vector<std::vector<T>>&& list) : list_{std::move(list)}, type_{type} {}
    graph_adj_list(graph_type type, std::initializer_list<std::initializer_list<T>> list) : list_{std::vector<std::vector<T>>(list.size() + 1)}, type_{type} {
        for (size_t i = 1; auto& l : list) {
            list_[i] = std::vector<T>(l);
            ++i;
        }
    }
    graph_type type() const {
        return type_;
    }
    size_t no_vertices() const {
        return list_.size() - 1;
    }
    size_t no_edges() const {
        size_t ret = 0;
        for (auto& v : list_) {
            ret += v.size();
        }
        return ret;
    }
    std::vector<T>& operator[](size_t i) {
        return list_[i];
    }
    const std::vector<T>& operator[](size_t i) const {
        return list_[i];
    }
};

template <typename T>
graph_adj_list<T> graph_read(std::istream& stream) {
    graph_type type;
    char mode;
    size_t no_vertices, no_edges;
    if (!(stream >> mode)) {
        throw file_format_exception{};
    }
    switch (mode) {
    case 'U':
        type = graph_type::undirected;
        break;
    case 'D':
        type = graph_type::directed;
        break;
    default:
        throw file_format_exception{};
    }
    if (!(stream >> no_vertices)) {
        throw file_format_exception{};
    }
    if (!(stream >> no_edges)) {
        throw file_format_exception{};
    }
    size_t u, v;
    std::vector<std::vector<T>> list(no_vertices + 1, std::vector<T>());
    for (size_t i = 0; i < no_edges && stream >> u && stream >> v; ++i) {
        list[u].push_back(v);
        if (type == graph_type::undirected) {
            list[v].push_back(u);
        }
    }
    if (stream.fail()) {
        throw file_format_exception{};
    }
    return graph_adj_list<T>(type, std::move(list));
}
template <typename T>
class pre_visit_order {
private:
    std::ostream& _out_stream;

public:
    explicit pre_visit_order(std::ostream& stream) : _out_stream{stream} {}
    void operator()(T elem) {
        _out_stream << elem << " ";
    }
};

template <typename T>
class empty_fun_obj {
public:
    empty_fun_obj() = default;
    void operator()(...) {}
};

template <typename T>
class tree_build {
private:
    std::vector<T> tree;

public:
    explicit tree_build(size_t no_vertices) : tree{std::vector<T>(no_vertices + 1, 0)} {}
    void operator()(T child, T parent) {
        tree[child] = parent;
    }
    const std::vector<T>& get_tree() const {
        return tree;
    }
};

template <typename T, typename pre_visit, typename post_visit, typename tree_build>
int graph_dfs(const graph_adj_list<T>& graph, T v, pre_visit& prev, post_visit& post, tree_build& build) {
    if (v < 1 || v > graph.no_vertices()) {
        return 1;
    }
    const size_t no_vertices = graph.no_vertices();
    size_t no_processed = 0;
    std::vector<int8_t> visited(no_vertices + 1, 0);
    std::stack<T> v_stack({v});
    while (no_processed < no_vertices) {
        if (v_stack.empty()) {
            T i = 1;
            for (; i <= no_vertices && (visited[i] != 0); ++i)
                ;
            v_stack.push(i);
        }
        T curr = v_stack.top();
        if (visited[curr] == 0) {
            visited[curr] = 1;
            prev(curr);
            for (auto e : graph[curr]) {
                if (visited[e] == 0) {
                    v_stack.push(e);
                    build(e, curr);
                }
            }
        } else {
            v_stack.pop();
            if (visited[curr] == 1) {
                post(curr);
                ++no_processed;
                visited[curr] = 2;
            }
        }
    }
    return 0;
}

template <typename T, typename procedure_t, typename post_procedure_t>
int graph_bfs(const graph_adj_list<T>& graph, T start_v, procedure_t& proc, post_procedure_t& post) {
    if (start_v < 1 || start_v > graph.no_vertices()) {
        return 1;
    }
    const size_t no_vertices = graph.no_vertices();
    size_t no_processed = 0;
    std::vector<int8_t> visited(no_vertices + 1, 0);
    std::queue<T> v_queue({start_v});
    visited[start_v] = 1;
    while (no_processed < no_vertices) {
        if (v_queue.empty()) {
            T i = 1;
            for (; i <= no_vertices && (visited[i] != 0); ++i)
                ;
            v_queue.push(i);
            visited[i] = 1;
        }
        T curr = v_queue.front();
        for (auto e : graph[curr]) {
            if (visited[e] == 0) {
                visited[e] = 1;
                proc(e, curr);
                v_queue.push(e);
            }
        }
        v_queue.pop();
        visited[curr] = 2;
        post(curr);
        ++no_processed;
    }
    return 0;
}

template <typename T>
class post_build_vertex_list {
private:
    std::vector<T> list;

public:
    explicit post_build_vertex_list() : list{std::vector<T>()} {}
    void operator()(T vertex) {
        list.push_back(vertex);
    }
    std::vector<T> get_list() {
        return list;
    }
};

template <typename T>
std::optional<std::vector<T>> graph_topological_sort(const graph_adj_list<T>& graph, T start_v = 1) {
    const size_t no_vertices = graph.no_vertices();
    size_t no_processed = 0;
    std::vector<int8_t> visited(no_vertices + 1, 0);
    std::stack<T> v_stack({start_v});
    std::vector<T> list(no_vertices);
    size_t head = no_vertices - 1;
    while (no_processed < no_vertices) {
        if (v_stack.empty()) {
            T i = 1;
            for (; i <= no_vertices && (visited[i] != 0); ++i)
                ;
            v_stack.push(i);
        }
        T curr = v_stack.top();
        if (visited[curr] == 0) {
            visited[curr] = 1;
            for (auto e : graph[curr]) {
                if (visited[e] == 0) {
                    v_stack.push(e);
                } else if (visited[e] == 1) {
                    return std::nullopt;
                }
            }
        } else {
            v_stack.pop();
            if (visited[curr] == 1) {
                list[head] = curr;
                --head;
                ++no_processed;
                visited[curr] = 2;
            }
        }
    }
    return list;
}

template <typename T>
std::vector<std::vector<T>> graph_strongly_connected(const graph_adj_list<T>& graph, T start_v = 1) {
    size_t no_vertices = graph.no_vertices();
    empty_fun_obj<T> empty_obj;
    post_build_vertex_list<T> list_builder;
    std::vector<std::vector<T>> list(no_vertices + 1, std::vector<T>());
    for (size_t i = 1; i <= no_vertices; ++i) {
        for (auto v : graph[i]) {
            list[v].push_back(i);
        }
    }
    graph_adj_list<T> r_graph(graph.type(), std::move(list));
    graph_dfs(r_graph, start_v, empty_obj, list_builder, empty_obj);
    std::vector<T> priority_stack = list_builder.get_list();
    std::vector<T> v_stack({priority_stack.back()});
    std::vector<std::vector<T>> components_list;
    std::vector<int8_t> visited(no_vertices + 1, 0);
    while (!priority_stack.empty()) {
        if (v_stack.empty()) {
            components_list.push_back(std::vector<T>());
            while (!priority_stack.empty() && visited[priority_stack.back()] != 0) {
                components_list.back().push_back(priority_stack.back());
                priority_stack.pop_back();
            }
            v_stack.push_back(priority_stack.back());
        }
        T curr = v_stack.back();
        if (visited[curr] == 0) {
            visited[curr] = 1;
            for (auto e : graph[curr]) {
                if (visited[e] == 0) {
                    v_stack.push_back(e);
                }
            }
        } else {
            v_stack.pop_back();
            visited[curr] = 2;
        }
    }
    return components_list;
}

template <typename T>
std::optional<std::vector<int8_t>> graph_bipart(const graph_adj_list<T>& graph, T start_v = 1) {
    const size_t no_vertices = graph.no_vertices();
    constexpr int8_t color_complement[3] = {0, 2, 1};
    int8_t curr_color = 1;
    size_t no_processed = 0;
    std::vector<int8_t> visited(no_vertices + 1, 0);
    std::queue<T> v_queue({start_v});
    visited[start_v] = curr_color;
    while (no_processed < no_vertices) {
        if (v_queue.empty()) {
            T i = 1;
            for (; i <= no_vertices && (visited[i] != 0); ++i)
                ;
            v_queue.push(i);
            visited[i] = 1;
        }
        T curr = v_queue.front();
        curr_color = visited[curr];
        for (auto e : graph[curr]) {
            if (visited[e] == curr_color) {
                return std::nullopt;
            } else if (visited[e] == 0) {
                visited[e] = color_complement[curr_color];
                v_queue.push(e);
            }
        }
        v_queue.pop();
        ++no_processed;
    }
    return visited;
}

#endif  // MY_GRAPHS_H
