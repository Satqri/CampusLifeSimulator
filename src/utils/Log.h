#ifndef CLS_CORE_LOG_H
#define CLS_CORE_LOG_H

#include <iostream>
#include <sstream>
#include <string>

namespace cls {
namespace log {

inline void output(const char* tag, const char* file, int line,
                   const char* func, const std::string& msg) {
    std::ostringstream oss;
    oss << "[" << tag << "] " << msg
        << "  (" << file << ":" << line << " " << func << "())";
    if (tag[0] == 'E')
        std::cerr << oss.str() << std::endl;
    else
        std::cout << oss.str() << std::endl;
}

} // namespace log
} // namespace cls

#define LOGI(msg) cls::log::output("INFO",  __FILE__, __LINE__, __func__, msg)
#define LOGW(msg) cls::log::output("WARN",  __FILE__, __LINE__, __func__, msg)
#define LOGE(msg) cls::log::output("ERROR", __FILE__, __LINE__, __func__, msg)

#endif
