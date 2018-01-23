#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace hlt {
    struct Log {
    private:
		std::vector<std::string> logs;
	
        std::ofstream file;

        void initialize(const std::string& filename) {
            file.open(filename, std::ios::trunc | std::ios::out);
        }
		
    public:
        static Log& get() {
            static Log instance{};
            return instance;
        }

        static void open(const std::string& filename) {
            get().initialize(filename);
        }
		
		static void clear() {
			get().logs.clear();
		}
		
		static void print() {
			for (std::string &s : get().logs) {
				std::cout << s << std::endl;
			}
		}

        static void log(const std::string& message) {
			get().logs.push_back(message);
            get().file << message << std::endl;
        }
    };
}
