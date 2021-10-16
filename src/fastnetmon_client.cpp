#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

//std::string cli_stats_file_path = "/tmp/fastnetmon.dat";
std::string cli_stats_file_path;
std::string global_config_path = "/etc/fastnetmon.conf";
std::string fastnetmon_version = "1.1";
typedef std::map<std::string, std::string> configuration_map_t;
configuration_map_t configuration_map;

// Load configuration
bool load_configuration_file() {
    std::ifstream config_file(global_config_path.c_str());
    std::string line;
    if (!config_file.is_open()) {
        //logger << log4cpp::Priority::ERROR << "Can't open config file";
        return false;
    }

    while (getline(config_file, line)) {
        std::vector<std::string> parsed_config;

        if (line.find("#") == 0 or line.empty()) {
            // Ignore comments line
            continue;
        }

        boost::split(parsed_config, line, boost::is_any_of(" ="), boost::token_compress_on);

        if (parsed_config.size() == 2 && parsed_config[0] == "cli_stats_file_path") {
            configuration_map[parsed_config[0]] = parsed_config[1];
            cli_stats_file_path = configuration_map["cli_stats_file_path"];
        }
    }
    return true;
}


int main(int argc, char **argv) {

    namespace po = boost::program_options;
    
    try {
            po::options_description desc("Allowed options");
            desc.add_options()
                    ("help", "produce help message")
                    ("version", "show version")
                    ("configuration_file", po::value<std::string>(), "set path to custom configuration file")
                    ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (vm.count("version")) {
            std::cout << "Version: " << fastnetmon_version << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (vm.count("configuration_file")) {
            global_config_path = vm["configuration_file"].as<std::string>();
            std::cout << "We will use custom path to configuration file: " << global_config_path << std::endl;
        }
    } catch (po::error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        exit(EXIT_FAILURE);
    }

    load_configuration_file();
    // Init ncurses screen
    initscr();

    // disable any character output
    noecho();

    // hide cursor
    curs_set(0);

    // Do not wait for getch
    timeout(0);

    while (true) {
        sleep(1);

        // clean up screen
        clear();

        int c = getch();

        if (c == 'q') {
            endwin();
            exit(0);
        }

        char* cli_stats_file_path_env = getenv("cli_stats_file_path");

        if (cli_stats_file_path_env != NULL) {
            cli_stats_file_path = std::string(cli_stats_file_path_env);
        } 

        std::ifstream reading_file;
        reading_file.open(cli_stats_file_path.c_str(), std::ifstream::in);

        if (!reading_file.is_open()) {
            std::cout << "Can't open fastnetmon stats file: " << cli_stats_file_path;
        }   

        std::string line = "";
        std::stringstream screen_buffer;
        while (getline(reading_file, line)) {
            screen_buffer << line << "\n";
        }

        reading_file.close();

        printw(screen_buffer.str().c_str());
        // update screen
        refresh();
    }

    /* End ncurses mode */
    endwin();
}
