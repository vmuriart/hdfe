<%
setup_pybind11(cfg)
%>
#include <pybind11/pybind11.h>
#include <algorithm>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

template <typename T>
pybind11::array_t<T> make_array(const std::vector<size_t>& shape) {
    return pybind11::array(pybind11::buffer_info(
        nullptr,
        sizeof(T),
        pybind11::format_descriptor<T>::value(),
        shape.size(),
        shape,
        calc_strides(shape, sizeof(T))
    ));
}

struct groupby {
    std::vector<std::vector<int>> indices;

    groupby(std::vector<int> keys) {
        auto result_it = std::max_element(keys.begin(), keys.end());
        int n_keys = *result_it + 1;
        indices.resize(n_keys);
        for (size_t i = 0; i < keys.size(); i++) {
            indices[keys[i]].push_back(i);
        }
    }

    std::vector<double> apply(std::vector<double> y, pybind11::object f) {
        std::vector<double> out(y.size());
        for (size_t i = 0; i < indices.size(); i++) {
            std::vector<double> group_input(indices[i].size());
            for (size_t j = 0; j < indices[i].size(); j++) {
                group_input[j] = y[indices[i][j]];
            }
            double group_output = f(group_input).cast<double>();
            for (size_t j = 0; j < indices[i].size(); j++) {
                out[indices[i][j]] = group_output;
            }
        }
        return out;
    }
    
    py::array_t<double> apply_2(py:array_t<double> y, pybind11::object f) {
        auto y_buf = y.request();
        py:array_t<double> out = make_array<double>({y.shape[0]});
        auto out_buf = out.request();
        for (size_t i = 0; i < indices.size(); i++) {
            std::vector<double> group_input(indices[i].size());
            for (size_t j = 0; j < indices[i].size(); j++) {
                group_input[j] = y_buf.ptr[indices[i][j]];
            }
            double group_output = f(group_input).cast<double>();
            for (size_t j = 0; j < indices[i].size(); j++) {
                out_buf.ptr[indices[i][j]] = group_output;
            }
        }
        return out;
    }

    void print_indices() {
        for (size_t i = 0; i < indices.size(); i++) {
            std::cout << "index " << i;
            for (int elt: indices[i]) {
                std::cout << " " << elt;
            }
            std::cout << std::endl;
        }
        
    }
};

PYBIND11_PLUGIN(cppplay) {
    pybind11::module m("cppplay", "auto-compiled c++ extension");
    pybind11::class_<groupby>(m, "Groupby")
        .def(pybind11::init<std::vector<int>>())
        .def("apply", &groupby::apply);
    return m.ptr();
}
