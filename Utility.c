#include "Utility.h"

static const char DELIMITER = '\n';

/* Read up to bufSize bytes or until delimiter, copying into the given
 * buffer as we go.
 * Encountering EOF after some data but before delimiter results in failure.
 * (That is: EOF is not a valid delimiter.)
 * Returns the number of bytes placed in buf (delimiter NOT transferred).
 * If buffer fills without encountering delimiter, negative count is returned.
 * If stream ends before first byte, -1 is returned.
 * Precondition: buf has room for at least bufSize bytes.
 */
int GetNextMsg(FILE *in, uint8_t *buf, size_t bufSize) {
  int count = 0;
  int nextChar;
  while (count < bufSize) {
    nextChar = getc(in);
    if (nextChar == EOF) {
      if (count > 0)
        FailedInUserError("GetNextMsg()", "Stream ended prematurely");
      else
        return -1;
    }
    if (nextChar == DELIMITER)
      break;
    buf[count++] = nextChar;
  }
  if (nextChar != DELIMITER) { // Out of space: count==bufSize
    return -count;
  } else { // Found delimiter
    return count;
  }
}

/* Write the given message to the output stream, followed by
 * the delimiter.  Return number of bytes written, or -1 on failure.
 */
int PutMsg(uint8_t buf[], size_t msgSize, FILE *out) {
  // Check for delimiter in message
  int i;
  for (i = 0; i < msgSize; i++)
    if (buf[i] == DELIMITER)
      return -1;
  if (fwrite(buf, 1, msgSize, out) != msgSize)
    return -1;
  fputc(DELIMITER, out);
  fflush(out);
  return msgSize;
}

size_t Encode(PostInfo *p,uint8_t *outBuf,size_t bufSize){
	if((p->isResponse && bufSize < sizeof(postBin)) || bufSize < 2* sizeof(uint16_t))
		Failed("Output buffer too small");
	postBin *po = (postBin *) outBuf;
	po->header = MAGIC;
	if(p->isInquiry)
		po->header |= INQUIRE_FLAG;
	else if(p->isResponse)
		po->header |= RESPONSE_FLAG;
	po->header = htons(po->header);
	po->userId = htons(p->userId);

	if(!p->isInquiry){
		po->postLen =(uint8_t) strlen(p->post);
		memcpy(po->post,p->post,po->postLen);
		return po->postLen + REQUEST_SIZE + 1;
	}else {
		return REQUEST_SIZE;
	}
}

bool Decode(uint8_t *inBuf, size_t mSize, PostInfo *p) {
	postBin *po = (postBin *) inBuf;
	uint16_t header = ntohs(po->header);
	if((mSize < REQUEST_SIZE )||(header & MAGIC_MASK) != MAGIC)
		return false;

	p->isResponse = ((header & RESPONSE_FLAG) != 0);
	p->isInquiry  = ((header & INQUIRE_FLAG)  != 0);
	p->userId = ntohs(po->userId);
	if(!p->isInquiry){
		uint8_t postLen = po->postLen;
		memcpy(p->post,po->post,postLen);
	}
	return true;
}

void FailedInUserError(const char*msg,const char *detail){
	fputs(msg,stderr);
	fputs(": ",stderr);
	fputs(detail,stderr);
	fputc('\n',stderr);
	exit(1);
}
void Failed(const char*msg){
	perror(msg);
	exit(1);
}
