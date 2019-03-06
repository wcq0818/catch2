#include <string.h>
#include <stdio.h>
#include <catch.hpp>
#include <iostream>

#pragma pack(push, 1)
typedef struct ST_TEST
{
	unsigned char	ucA : 1;
	unsigned char	ucB : 1;
	unsigned char	ucC : 1;
	unsigned char	ucD : 1;
	unsigned char	ucE : 1;
	unsigned char	ucF : 1;
	unsigned char	ucG : 1;
	unsigned char	ucH : 1;
} ST_TEST;
#pragma pack(pop)

TEST_CASE("test_struct_bit", "test_struct_bit")
{
	ST_TEST stTest;
	stTest.ucA = 1;
	stTest.ucB = 0;
	stTest.ucC = 0;
	stTest.ucD = 0;
	stTest.ucE = 0;
	stTest.ucF = 0;
	stTest.ucG = 1;
	stTest.ucH = 0;

	unsigned char ucTest;
	memcpy(&ucTest, &stTest, 1);
	//没有现成的打印二进制的方法，所以用16进制打印
	printf("%x", ucTest);

	//scanf("%c", &ucTest);
}

enum Priority
{
	ABOVE_HIGH = 0x10,
	HIGH = 0x20,
	ABOVE_NORMAL = 0x30,
	NORMAL = 0x40,
	BELOW_NORMAL = 0x50,
	LOW = 0x60,
};

#pragma pack(push, 1)
struct frame_header_struct
{
	unsigned char frame_start;
// 	unsigned int sender_id;
// 	unsigned char sender_type;
// 	unsigned char protocol_type;
	union
	{
		struct
		{
			unsigned char sequence : 4,
						  priority : 4;
		};
		unsigned char frame_seq;
	};
	unsigned char event_type;
	unsigned short int event_sub_type;
	unsigned int transfer_id;
	unsigned char transfer_type;
	unsigned int receiver_id;
	unsigned char receiver_type;
	unsigned short int data_length;
	unsigned char frame_end;
};
#pragma pack(pop)

TEST_CASE("frame_header_struct", "frame_header_struct")
{
	frame_header_struct data;

	memset(&data, 0, sizeof(frame_header_struct));
	data.frame_start = 0xFF;
// 	data.frame_seq = NORMAL | 0;
// 	if (((Priority)(data.frame_seq & 0xF0)) == NORMAL)
// 	{
// 		std::cout << data.frame_seq <<std::endl;
//	}
	data.sequence = 0xF;
}