//
// Created by fenix on 2/25/20.
//

#ifndef LAB_2_PARALLEL_INTEGRATION_CONFIGFILEOPT_H
#define LAB_2_PARALLEL_INTEGRATION_CONFIGFILEOPT_H


#include <string>
#include <boost/program_options.hpp>


class ConfigFileOpt {
public:
    ConfigFileOpt();

    ~ConfigFileOpt() = default;

    void parse(const std::string &file_name);

    [[nodiscard]] double get_abs_pars() const { return abs_pars; }

    [[nodiscard]] double get_rel_pres() const { return rel_pres; }

    [[nodiscard]] uint16_t get_flow_num() const { return flow_num; }

    [[nodiscard]] size_t get_init_steps() const { return init_steps; }

    [[nodiscard]] size_t get_max_steps() const { return max_steps; }

    [[nodiscard]] size_t get_m() const { return m; }

    [[nodiscard]] std::vector<double> get_a1() const { return a1; }

    [[nodiscard]] std::vector<double> get_a2() const { return a2; }

    [[nodiscard]] std::vector<double> get_c() const { return c; }

    [[nodiscard]] std::pair<double, double> get_x() const { return x; }

    [[nodiscard]] std::pair<double, double> get_y() const { return y; }

private:
    void init_opt_description();

    static std::string assert_file_exist(const std::string &f_name);

    void assert_valid_opt_vals() const;

    double abs_pars = 0.0, rel_pres = 0.0;
    size_t init_steps = 0, max_steps = 0, m = 0;
    uint16_t flow_num = 1;

    std::vector<double> a1{}, a2{}, c{};
    std::pair<double, double>
            x{0.0, 0.0};
    std::pair<double, double> y{0.0, 0.0};

    boost::program_options::options_description opt_conf{"Config File Options"};
    boost::program_options::variables_map var_map{};
};


ConfigFileOpt parse_args(int argc, char **argv);

void assert_valid_config(const ConfigFileOpt &conf);

#endif //LAB_2_PARALLEL_INTEGRATION_CONFIGFILEOPT_H
