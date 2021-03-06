#include <iostream>
#include <string>
#include <boost/mpi.hpp>
#include <filesystem>

#include "files/config_file.h"
#include "mpi_program.h"
#include "speed_tester.h"
#include "code_control.h"


ConfigFileOpt parse_args(int argc, char **argv);

void assert_valid_config(const ConfigFileOpt &conf);

int main(int argc, char *argv[]) {
    auto start = get_current_time_fenced();
    ConfigFileOpt config = parse_args(argc, argv);
    assert_valid_config(config);

    boost::mpi::environment env{argc, argv};
    boost::mpi::communicator world{};
    const int workers_num = world.size() - 1;

    if (!workers_num) {
        std::cerr << "ERROR: the number of tasks must be at least 1.\nQuitting..." << std::endl;
        return 1;
    }

    if (world.rank() == MASTER_ID)
        master_code(world, config);
    else
        slave_code(world, config);

#ifdef TIME
    const auto finish_time = get_current_time_fenced();
    std::cout << "Total " << world.rank() << " time: " << to_us(finish_time - start) << std::endl;
#endif
#ifdef DEBUG
    std::cout << "FINISH process with rank " << world.rank() << " of " << world.size() << std::endl;
#endif
    return 0;
}

ConfigFileOpt parse_args(int argc, char **argv) {
    //  ##################### Program Parameter Parsing ######################
    std::string filename = "config.conf";
    if (argc == 2) {
        filename = argv[1];
    } else if (argc > 2) {
        std::cerr << "Too many arguments. Usage: \n"
                     "\tprogram [config-filename]\n" << std::endl;
        exit(1);
    }
    //  #####################    Config File Parsing    ######################
    ConfigFileOpt config;
    try {
        config.parse(filename);
    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(3);
    }
    return config;
}

void assert_valid_config(const ConfigFileOpt &config) {
    // TODO: rewrite through exceptions
    if (!std::filesystem::exists(config.get_field_filename())) {
        std::cerr << "Error: File or Directory '" << config.get_field_filename()
                  << "' do not exist (or can not be created)!"
                  << std::endl;
        exit(21);
    } else if (config.get_field_filename().empty()) {
        std::cerr << "Error: Field file is empty or missing field file filename!" << std::endl;
        exit(23);
    } else if (config.get_delta_t() >=
               std::pow(std::max(config.get_delta_x(), config.get_delta_y()), 2) / config.get_alpha() / 4) {
        std::cerr << "Error: Violation of the Von Neumann criteria for input data." << std::endl;
        exit(23);
    } else if (config.get_width() < 1000 or config.get_height() < 1000) {
        std::cerr << "Error: field must be at least 1000*1000" << std::endl;
        exit(23);
    }
}
