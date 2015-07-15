/*
 * libpipecolors: linux color code library
 *
 * Authors:
 * Eric Wheeler <eric@ericwheeler.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef PC_REMOVE_INVALID
#define PC_REMOVE_INVALID false
#endif

#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <boost/regex.hpp>
#include "pipecolors.h"

namespace pipecolors {

  const char * ansi(std::string code) {

    char * buf;
    int sret = asprintf(&buf, "%c%c", code[1], code[2]);
    int pc = atoi( buf );
    delete buf;

    if(sret == -1) {
      return "failure";
    }

    switch(pc) {
      case 0:  return "\x1b[0;30m"; break; // FG_BLACK
      case 1:  return "\x1b[0;34m"; break; // FG_BLUE
      case 2:  return "\x1b[0;32m"; break; // FG_GREEN
      case 3:  return "\x1b[0;36m"; break; // FG_CYAN
      case 4:  return "\x1b[0;31m"; break; // FG_RED
      case 5:  return "\x1b[0;35m"; break; // FG_MAGENTA
      case 6:  return "\x1b[0;33m"; break; // FG_YELLOW
      case 7:  return "\x1b[0;37m"; break; // FG_GRAY
      case 8:  return "\x1b[0;90m"; break; // FG_GRAY_D
      case 9:  return "\x1b[0;94m"; break; // FG_BLUE_L
      case 10: return "\x1b[0;92m"; break; // FG_GREEN_L
      case 11: return "\x1b[0;96m"; break; // FG_CYAN_L
      case 12: return "\x1b[0;91m"; break; // FG_RED_L
      case 13: return "\x1b[0;95m"; break; // FG_MAGENTA_L
      case 14: return "\x1b[0;93m"; break; // FG_YELLOW_L
      case 15: return "\x1b[0;97m"; break; // FG_WHITE

      case 16: return "\x1b[1;40m"; break; // BG_DEFAULT
      case 17: return "\x1b[1;44m"; break; // BG_BLUE
      case 18: return "\x1b[1;42m"; break; // BG_GREEN
      case 19: return "\x1b[1;46m"; break; // BG_CYAN
      case 20: return "\x1b[1;41m"; break; // BG_RED
      case 21: return "\x1b[1;45m"; break; // BG_MAGENTA
      case 22: return "\x1b[1;43m"; break; // BG_YELLOW
      case 23: return "\x1b[1;47m"; break; // BG_WHITE

      case 30: return "\x1b[1m";    break; // Bold On
      case 31: return "\x1b[0m";    break; // Bold Off
      case 39: return "\x1b[0;39m"; break; // FG_DEFAULT
      default: return "nocode";     break;
    }
  }

  bool has_colors(void) {
    return isatty(fileno(stdout));
  }

  std::pair<std::string,int> replace_colors( std::string s) {

    using namespace boost;

    regex re( "(\\|\\d\\d)" );
    match_results<std::string::const_iterator> match;
    match_flag_type flags = boost::match_default;

    std::string::const_iterator start, end;
    start = s.begin();
    end = s.end();
    std::string len(s);

    while(regex_search(start, end, match, re, flags))
    {

      if(ansi(match[0].str()) == "nocode" && PC_REMOVE_INVALID == false) goto skip;

        len.erase(len.find(match[0]), match[0].length());

      if(has_colors() && ansi(match[0].str()) != "nocode") {
          s.replace(s.find(match[0]), match[0].length(), ansi(match[0].str()) );
      } else {
        s.erase(s.find(match[0]), match[0].length());
      }

      skip:;
      start = match[0].second;
      flags |= boost::match_prev_avail | boost::match_not_bob;
    }

    return std::make_pair(s, len.length());

  }

  int pcprintf( const char * format, ...)
  {
    char * buffer;
    va_list args;
    int ret;
    std::pair<std::string, int> result;

    va_start(args, format);
    ret = vasprintf(&buffer, format, args);
    va_end(args);
    if(ret == -1) {
        free(buffer);
        exit(EXIT_FAILURE);
    }

    std::string s(buffer);
    free(buffer);

     result = replace_colors(s);

    printf("%s", result.first.c_str());

    return(result.second);

  }


  int pcsprintf( char * str, const char * format, ... ) {

    int ret;
    char** str1 = (char**)str;
    va_list args;

    va_start(args, format);
    ret = vasprintf(str1, format, args);
    va_end(args);

    if(ret == -1) {
        free(str1);
        exit(EXIT_FAILURE);
    }

    std::string s(*str1);
    free(*str1);
    std::pair<std::string, int> result = replace_colors(s);
    const char * s2 = (const char *)result.first.c_str();
    strcpy(str, s2);
    return result.second;
  }
} // namespace