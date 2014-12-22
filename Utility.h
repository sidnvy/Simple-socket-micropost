#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAXIMUM_USERID 1024
#define MAX_WIRE_SIZE 150 
#define MAX_POST_LENGTH 128

struct PostInfo {
	uint16_t userId;
	bool isInquiry;
	bool isResponse;
	char post[MAX_POST_LENGTH];
};

typedef struct PostInfo PostInfo;

enum {
  REQUEST_SIZE = 4,
  RESPONSE_SIZE = 12,
  MAX_SIZE = 133,
  INQUIRE_FLAG = 0x0100,
  RESPONSE_FLAG = 0x0200,
  MAGIC = 0x5400,
  MAGIC_MASK = 0xfc00
};

struct postBin{
	uint16_t header;
	uint16_t userId;
	uint8_t  postLen;
	uint8_t post[MAX_POST_LENGTH];
};

typedef struct postBin postBin;


void Failed(const char*msg);
void FailedInUserError(const char*msg,const char *detail);
size_t Encode(PostInfo *v, uint8_t *outBuf, size_t bufSize);
bool Decode(uint8_t *inBuf, size_t mSize, PostInfo *v);
int GetNextMsg(FILE *in, uint8_t *buf, size_t bufSize);
int PutMsg(uint8_t buf[], size_t msgSize, FILE *out);