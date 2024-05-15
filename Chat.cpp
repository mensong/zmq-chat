//  Decentralized chat example
//
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <process.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

//arg-192.168.1.
static unsigned __stdcall listener_chat_task (void *arg)
{
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_SUB);

    char uri[100];
    int address;
    for (address = 1; address < 255; address++) 
    {
        sprintf (uri, "tcp://%s%d:9000", (char *) arg, address);
        int rc = zmq_connect (requester, uri);
        if (rc == -1)
            printf ("Connect %s%d error\n", (char *) arg, address);
    }

    const char *channel = "";
    int channelLen = strlen (channel);
    zmq_setsockopt (requester, ZMQ_SUBSCRIBE, channel, channelLen);

    char buffer[1024];
    int buffSize = sizeof (buffer) - 1;
    buffer[buffSize] = '\0';
    while (1) 
    {
        int recvLen = zmq_recv (requester, buffer, buffSize, 0);
        if (recvLen != -1)
        {
            buffer[min (recvLen, buffSize)] = '\0';
            printf ("%s\n", buffer);
        }
    }

    zmq_close (requester);
    zmq_ctx_destroy (context);

    return 0;
}

int main (int argc, char *argv[])
{
    char *dot = argc == 4 ? strrchr (argv[1], '.') : NULL;
    if (!dot || argc != 4)
    {
        puts ("Usage: Char ipaddress interface username");
        puts ("Example: Char 192.168.77.128 * joe");
        exit (0);
    }

    unsigned int uiThreadID = 0;        
    if (dot) dot[1] = 0; //  Cut string after dot
    _beginthreadex (NULL, 0, listener_chat_task, (void *) argv[1], 0, &uiThreadID);
    
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_PUB);
    char uri[100];
    sprintf (uri, "tcp://%s:9000", argv[2]);
    int rc = zmq_bind (responder, uri);
    if (rc != 0) 
    {
        zmq_ctx_destroy (context);
        return 0;
    }

    const char *nickName = argv[3];
    int nickNameLen = strlen(nickName);

    char txt[1024];
    char message[1024];
    while (1) 
    {
        if (!fgets (txt, sizeof (txt) - 1, stdin))
            break;
        //remove \n
        txt[strlen (txt) - 1] = '\0';

        sprintf (message, "%s: %s", nickName, txt);
                
        zmq_send (responder, message, strlen (message), 0);
    }

    zmq_close (responder);
    zmq_ctx_destroy (context);
    return 0;
}