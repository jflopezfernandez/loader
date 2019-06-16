
#ifndef PROJECT_INCLUDES_pba_HPP_
#define PROJECT_INCLUDES_pba_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <locale.h>
#include <limits.h>
#include <inttypes.h>

#include <algorithm>
#include <any>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <locale>
#include <memory>
#include <new>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>

namespace Options = boost::program_options;

#include <bfd.h>

#include <loader.hpp>

#if !defined(FALSE) || !defined(TRUE)
enum { FALSE = 0, TRUE = !FALSE };
#endif // FALSE || TRUE

#endif // PROJECT_INCLUDES_pba_HPP_

