#include "data_def.h"
#include<unistd.h>
#include<iconv.h> 
#include <fcntl.h>

void ToJson(const Props_type & props, json_spirit::Value & value)
{
	json_spirit::Array items;

	for (Props_type::const_iterator it = props.begin(); it != props.end(); ++it)
	{
		json_spirit::Array item;
		item.push_back(it->pid);
		item.push_back(it->pcate);
		item.push_back(it->pframe);
		item.push_back(it->status);
		item.push_back(it->sptime);
		item.push_back(it->sltime);
		item.push_back(it->sendmid);

		items.push_back(item);
	}

	value = items;
}

std::string GBKToUTF8(const std::string& strGBK)
{
#ifdef _WIN32
	wchar_t* str1;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	str1 = new wchar_t[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	std::string strOutUTF8(str2);
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return strOutUTF8;
#else
	if (strGBK.empty())
	{
		return "";
	}
	iconv_t cd = iconv_open("UTF-8", "GB18030");
	size_t inlen = strGBK.length();
	char *outbuf = (char *)malloc(inlen * 4);
	bzero(outbuf, inlen * 4);
	const char *in = strGBK.c_str();
	char *out = outbuf;
	size_t outlen = inlen * 4;
	if (iconv(cd, (char**)&in, (size_t *)&inlen, &out, &outlen) == (size_t)-1)
	{
        int nErr = errno;
        switch (nErr)
        {
        case E2BIG:
        {
            printf("errno:E2BGI（OutBuf空间不够）\n");
            break;
        }
        case EILSEQ:
        {
            printf("errno:EILSEQ（InBuf多字节序无效）\n");
            break;
        }
        case EINVAL:
        {
            printf("errno:EINVAL（有残留的字节未转换）\n");
            break;
        }
        default:
            break;
        }
        free(outbuf);
        iconv_close(cd);
        return "";
	}
	else
	{
		outlen = strlen(outbuf);
		std::string strOutUTF8(outbuf);
		free(outbuf);
		iconv_close(cd);
		return strOutUTF8;
	}
#endif
}

