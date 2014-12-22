#include "Utility.h"


int main(int argc, char *argv[]){
	// Argument validation
	if(argc < 4 || argc > 5)
		FailedInUserError("Parameter(s)","<Ip> <Port> <UserId> [Post] ");
	bool inq = (argc == 4);
	char *msg = argv[4];
	if(sizeof(msg) > MAX_POST_LENGTH)
		FailedInUserError("Message is too long:","Can't longer than 128 Bytes");
	uint16_t ID =(uint16_t) atoi(argv[3]);
	if(ID > MAXIMUM_USERID)
		FailedInUserError("User ID is too large:","Maximum UserID is 1024");

	// Create socket and connect
	int sock = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in servAddr;
	memset(&servAddr,0,sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(atoi(argv[2]));
	int rtv = inet_pton(AF_INET,argv[1],&servAddr.sin_addr.s_addr);
	if(rtv == 0)
		FailedInUserError("inet_pton() failed","invalid address string");
	else if(rtv < 0)
		Failed("inet_pton() failed");
	if(connect(sock,(struct sockaddr*)&servAddr,sizeof(servAddr)) < 0)
		Failed("connect()failed");

	// Wrap in stream
	FILE *str = fdopen(sock,"r+");
	if(str == NULL)
		Failed("fdopen() failed");

	// Construct Post
	PostInfo p;
	memset(&p,0,sizeof(p));
	p.isInquiry = inq;
	p.isResponse = false;
	p.userId = ID;
	if(!p.isInquiry)
		memcpy(&p.post,msg,strlen(msg));

	uint8_t outbuf[MAX_WIRE_SIZE];
	size_t codedSize = Encode(&p,outbuf,MAX_WIRE_SIZE);

	// Console log Send and Recv
	printf("Sending %d-bytes %s for User %d ...\n",(int) codedSize,
		(inq ? "inquiry" : "post"),ID);
	if(PutMsg(outbuf,codedSize,str) < 0)
		Failed("PutMsg() failed");
	uint8_t inbuf[MAX_WIRE_SIZE];
	size_t uncodedSize = GetNextMsg(str,inbuf,MAX_WIRE_SIZE);

	// Decode and print corresponding info
	if(Decode(inbuf,uncodedSize,&p)){
		printf("Received:\n");
		if(p.isResponse)
			printf("Response to ");
		if(p.isInquiry)
			printf("inquiry ");
		else
			printf("publish ");
		printf("for user %d \n",p.userId);
		if(p.isResponse)
			printf("micropost is: %s\n",p.post);	
	}
	fclose(str);

	return 0;
}