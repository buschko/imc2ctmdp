#ifndef __IMC2CTMDP_H
#define __IMC2CTMDP_H

//#define NODEBUG
//#define NOWARN

//#define NOTIMING

// some helper functions
// countTime: print the given text together with the elapsed time since the
//            last call
void countTime(const char* text);
// getFormat: get the format of the given filename and the real filename
std::pair<std::string, std::string> getFormat(const std::string &filename,
    const std::string &default_filename);

#endif

