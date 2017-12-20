#ifndef _CCLL_BASE64_H_
#define _CCLL_BASE64_H_

#include <string>

std::string base64_encode(unsigned char const*, unsigned int len);
std::string base64_decode(std::string const& s);

#endif //_CCLL_BASE64_H_
