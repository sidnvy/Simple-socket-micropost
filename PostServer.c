#include "Utility.h"

static const int MAXPENDING = 5;
static char *postDatabase[MAXIMUM_USERID+1][MAX_POST_LENGTH];

int main(int argc, char *argv[]){

	// Argument validation
	if (argc != 2)
		FailedInUserError("Parameter(s)", "<Server Port/Service>");

	// Socket() and Bind() and Listen()
	struct sockaddr_in servAddr;
	memset(&servAddr,0,sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));
	int servSock = socket(AF_INET,SOCK_STREAM,0);
	if(servSock < 0)
		Failed("socket() failed");
	if(bind(servSock,(struct sockaddr *)&servAddr,sizeof(servAddr)) < 0)
		Failed("bind() failed");
	if(listen(servSock,MAXPENDING) < 0)
		Failed("listen() failed");

	// Run forever
	for (;;) { 
		// Accept connections
		struct sockaddr_in clntAddr;
		socklen_t clntAddrLen = sizeof(clntAddr);
	    int clntSock = accept(servSock,(struct sockaddr *)&clntAddr,&clntAddrLen);
	    if(clntSock < 0)
	    	Failed("accept() failed");

	    // Create an input stream from the socket
	    FILE *channel = fdopen(clntSock, "r+");
	    if (channel == NULL)
	    	Failed("fdopen() failed");

	    // Get from stream and analysis
	    PostInfo p;
	    uint8_t inbuf[MAX_WIRE_SIZE];
	    int mSize;
	    bool inq;
	    while((mSize = GetNextMsg(channel,inbuf,MAX_WIRE_SIZE)) > 0){
	    	memset(&p,0,sizeof(p));
	    	printf("Received message (%d bytes)\n",mSize);
	    	if(Decode(inbuf,mSize,&p)){
	    		if(!p.isResponse){
	    			p.isResponse = true;
	    			if(p.userId >= 0 && p.userId <= MAXIMUM_USERID){
	    				if(!p.isInquiry){
	    					printf("Is a publish\n");
	    					memset(postDatabase[p.userId],0,MAX_POST_LENGTH);
	    					memcpy(postDatabase[p.userId],p.post,strlen(p.post));
	    				}
	    				memcpy(p.post,postDatabase[p.userId],
	    					sizeof(postDatabase[p.userId]));
	    				inq = p.isInquiry;
	    				p.isInquiry = false;
	    			}
	    		}
	    		uint8_t outbuf[MAX_WIRE_SIZE];
	    		mSize = Encode(&p,outbuf,MAX_WIRE_SIZE);
	    		if(PutMsg(outbuf,mSize,channel) < 0){
	    			fputs("Error framing/outputting message\n",stderr);
	    			break;
	    		}else{
	    			printf("Processed %s for user %d\n",
	    				(inq ? "inquiry":"publish"),p.userId);
	    		}
	    		fflush(channel);
	    	}else{
	    		fputs("Parse error,closing connections.\n",stderr);
	    		break;
	    	}
	    }
	    puts("Client finished");
	    fclose(channel);
	}
	// Not reached
	return 0;
}