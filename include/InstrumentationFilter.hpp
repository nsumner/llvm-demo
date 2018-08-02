#ifndef INSTRUMENTATION_FILTER_HPP
#define INSTRUMENTATION_FILTER_HPP

#include "json11.hpp"

#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <fstream>
#include <map>

using namespace std;
using namespace json11;
typedef enum {FN_SIZE_LOC, FN_SIZE_IR, FN_SIZE_PATH} fn_size_metrics;

class InstrumentationFilter {
    private:
        Json filters;
        bool loaded;
        string fname;
        map<string, int> functions;

        bool globMatch(string glob, string s);
        void testGlob();

        bool checkFunctionBlacklist(string function_name);
        bool checkFileBlacklist(string file_name);
        string findBestFunctionMatch(string function_name);

    public:

        InstrumentationFilter() : loaded(false) {
            string filename("function_sizes.json");
            const char * val = ::getenv("DIN_MAPS");
            string prefix = "./";
            if ((val == 0) || (strcmp(val,"") == 0)) {
                cerr << "DIN_MAPS not set, falling back to current directory" << endl;
            } else {
                prefix = val;
                prefix += "/";
            }

            ifstream mapin;
            fname = prefix + filename;
            mapin.open(fname);
            if (!mapin.is_open()) {
                return;
            }
            stringstream ss;
            string err;
            ss << mapin.rdbuf();
            json11::Json jsmap;
            jsmap = json11::Json::parse(ss.str(), err);
            for (auto entry : jsmap.object_items()) {
                functions[entry.first] = entry.second.int_value();
            }
        };

        ~InstrumentationFilter() {
            ofstream outfile;
            outfile.open(fname, ofstream::trunc);
            json11::Json jsmap(functions);
            outfile << jsmap.dump();
            outfile.flush();
            outfile.close();

        }
        void loadFilterData(const char *filename);
        void loadFilterDataEnv();

        bool loopCheckEnabled();

        bool checkFunctionFilter(string function_name, string event_type);
        bool checkFunctionArgFilter(string function_name, int arg);
        bool checkFileFilter(string file_name);
        bool checkFunctionSize(string function_name, size_t size);
        fn_size_metrics getFunctionSizeMetric();
};

#endif
