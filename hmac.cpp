#include "catch.hpp"
#include "algo_hmac.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <openssl/hmac.h>
#include "tools/common.h"

using namespace std;

TEST_CASE("HMAC_SHA256", "[HMAC_SHA256]")
{
	std::string plain_text = "hello world";

	unsigned char ak[] = { 0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF };
	std::string oct_ak = Common::Util_tools::bytes_to_hex(ak, 16);
	std::string auth_tag;
	unsigned char* output = (unsigned char*)malloc(EVP_MAX_MD_SIZE);
	memset(output, 0, EVP_MAX_MD_SIZE);
	if (output != nullptr)
	{
		unsigned int output_length = 0;

		HMAC_CTX ctx;
		HMAC_CTX_init(&ctx);
		HMAC_Init_ex(&ctx, oct_ak.c_str(), oct_ak.length(), EVP_sha256(), NULL);
		HMAC_Update(&ctx, (unsigned char*)plain_text.c_str(), plain_text.length());        // input is OK; &input is WRONG !!!

		HMAC_Final(&ctx, output, &output_length);
		HMAC_CTX_cleanup(&ctx);

		auth_tag = (char*)output;
		free(output);
	}
	std::cout << auth_tag << std::endl;
}

TEST_CASE("hmac", "[hmac][.\hide]")
{
	char key[] = "012345678";
	string data = "hello world";

	unsigned char * mac = NULL;
	unsigned int mac_length = 0;

	int ret = HmacEncode("sha256", key, strlen(key), data.c_str(), data.length(), mac, mac_length);

	if (0 == ret) 
	{
		cout << "Algorithm HMAC encode succeeded!" << endl;
	}
	else 
	{
		cout << "Algorithm HMAC encode failed!" << endl;
	}

	cout << "mac length: " << mac_length << endl;
	cout << "mac:";
	for (int i = 0; i < mac_length; i++) 
	{
		printf("%-03x", (unsigned int)mac[i]);
	}
	cout << endl;

	if (mac) 
	{
		free(mac);
		cout << "mac is freed!" << endl;
	}
}
