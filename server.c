#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket
#include <arpa/inet.h>  //for inet_addr
#include <pthread.h>    //for mutex lock
#include <grp.h>
#include <pwd.h>
#include "mainheader.h"

//declaration of functions
void getGroupIDs(char *user);
void alterIDs(int myid);
void getFile(int socketConnection);
void selectedFile();
void *serverHandler(void *fd_pointer); //server handler function to handle each thread coming in
void test();

//globals
int userId = 0;
//temp array to hold user gid's
gid_t myGroupIds[50] = {};
//user to be looked for
char searchedUser[100];
//selected directory for file transfers
char searchedFolder[100];
//used for mutex lock for shared resources
pthread_mutex_t lock_x;
//global vars set to be used inside the thread handler to determine which user has left
struct sockaddr_in clientAddress, serverAddress;

int socketConnection;
#define PORT 8082

int main()
{
    int cs; //client socket
    int *newSocket;
    socklen_t connSize;
    int s = socket(AF_INET, SOCK_STREAM, 0); //socket descriptor

    // create a mutex lock*
    pthread_mutex_init(&lock_x, NULL);

    if (s == -1)
    {
        printf("Could not create socket.\n");
        exit(1);
    }
    else
    {
        printf("Socket Successfully Created!!.\n");
    }

    //set sockaddr_in variables
    serverAddress.sin_port = htons(PORT); // Set the prot for communication
    serverAddress.sin_family = AF_INET;   // Use IPV4 protocol
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    // When INADDR_ANY is specified in the bind call, the  socket will  be bound to all local interfaces

    //Bind
    if (bind(s, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Bind issue!!");
        return 1;
    }
    else
    {
        printf("Bind to Port: %d Complete!!\n", PORT);
    }

    if (listen(s, 10) == 0)
    {
        //accept an incoming connection
        printf("Listening....\n");
    }
    else
    {
        //mark listener error
        printf("!!Error Encountered While Listening!!\n");
    }

    printf("Waiting for incomming connection from Client>>\n");
    connSize = sizeof(clientAddress);

    while ((cs = accept(s, (struct sockaddr *)&clientAddress, &connSize)))
    {
        printf("Connection from %s:%d accepted!!\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        // Create a new thread to handle each incomming client connection
        pthread_t serverThread;
        // Allocate the right amount of bytes required for the socket (instead of a hard coded integer)
        newSocket = malloc(sizeof *newSocket);
        // Using client socket
        *newSocket = cs;
        // create each new thread with the server handler function
        pthread_create(&serverThread, NULL, serverHandler, (void *)newSocket);
    }

    if (cs < 0)
    {
        perror("!!Connection Failed!!");
        return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

//Some code from notes for assignment: FileTransferServer.txt
void *serverHandler(void *fd_pointer)
{
    printf("Inside thread finction with ID : %lu\n", pthread_self());

    int readSize;
    static char clientMessage[2000];
    socketConnection = *(int *)fd_pointer;

    while (1)
    {
        //receive clientMessage
        readSize = recv(socketConnection, clientMessage, 2000, 0);

        if (readSize == 0)
        {
            //printf("Error receeiving clientMessage\n");
            memset(clientMessage, '\0', 2000); // free the buffer
            fflush(stdout);
            break;
        }
        else
        {
            printf("Message Sent From client is: %s \n", clientMessage);

            // Separate the  string received from the client based on the white space separation (path and name)
            char nameAndDirectory[100][100];
            int i;
            int j = 0;
            int counter = 0;

            for (i = 0; i <= (strlen(clientMessage)); i++)
            {
                // if space found, assign NULL into nameAndDirectory[counter]
                if (clientMessage[i] == ' ')
                {
                    nameAndDirectory[counter][j] = '\0';
                    counter++; //for next word
                    j = 0;     //for next word, init index to 0
                }
                else
                {
                    nameAndDirectory[counter][j] = clientMessage[i];
                    j++;
                }
            }
            printf("Separating Client Message...\n");

            // Assign directory and username to new vars
            strcpy(searchedFolder, nameAndDirectory[0]);
            strcpy(searchedUser, nameAndDirectory[1]);

            printf("Folder: %s\n", searchedFolder);
            printf("Username: %s\n", searchedUser);
            // free the buffer
            memset(clientMessage, '\0', 2000);

            // Then receive file
            char file_buffer[512]; // Receiver buffer
            char *fr_name = "/tmp/test.txt";

            //******************************************************************
            //***************************************************************
            FILE *fr = fopen(fr_name, "w");
            if (fr == NULL)
            {
                printf("File %s Cannot be opened file on server.\n", fr_name);
            }
            else
            {
                bzero(file_buffer, 512);
                int fr_block_sz = 0;
                int i = 0;
                //while ((fr_block_sz = recv(cs, revbuf, LENGTH, 0)) > 0)
                while ((fr_block_sz = recv(socketConnection, file_buffer, 512, 0)) > 0)
                {
                    printf("Data Received %d = %d\n", i, fr_block_sz);
                    int write_sz = fwrite(file_buffer, sizeof(char), fr_block_sz, fr);
                    if (write_sz < fr_block_sz)
                    {
                        perror("File write failed on server.\n");
                    }
                    bzero(file_buffer, 512);
                    i++;
                }
            }
            printf("Ok received from client!\n");
            fclose(fr);

            // call get Group ID's
            getGroupIDs(searchedUser);
        }
        // free the buffer
        memset(clientMessage, '\0', 2000);
    }
    if (readSize == 0)
    {
        printf("%s:%d timed out\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        fflush(stdout);
    }

    /*else if(readSize == -1)
	{
    	fprintf(stderr, "recv() failed due to errno = %d\n", errno);
	}*/
    else if (readSize == -1)
    {
        perror("recv failed");
    }

    // Free pointer
    free(fd_pointer);
    // Close socket
    close(socketConnection);
    // Close thread
    pthread_exit(NULL);

    return 0;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

void getGroupIDs(char *user) //complete
//code from http://man7.org/linux/man-pages/man3/getgrouplist.3.html to handle getting the group id's
{
    int j;
    int ngroups = 50;
    gid_t *groups;
    struct passwd *pw;
    struct group *gr;

    groups = malloc(ngroups * sizeof(gid_t));

    if (groups == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // getpwnam = search user database to get passwd structure (contains first group ID for user)
    // (returns null pointer and errno is 0 if error occurs)
    pw = getpwnam(user);
    if (pw == NULL)
    {
        perror("getpwnam");
        exit(EXIT_SUCCESS);
    }

    // getgrouplist - get list of groups to which a user belongs
    if (getgrouplist(user, pw->pw_gid, groups, &ngroups) == -1)
    {
        fprintf(stderr, "getgrouplist() returned -1; ngroups = %d\n",
                ngroups);
        exit(EXIT_FAILURE);
    }

    /* Display list of retrieved groups, along with group names */
    for (j = 0; j < ngroups; j++)
    {
        //printf("%d", groups[j]);
        myGroupIds[j] = groups[j]; // add each group ID to array so we can search for them later.
        gr = getgrgid(groups[j]);
        if (gr != NULL)
            printf(" (%s)", gr->gr_name);
        printf("\n");
    }

    // from the retrieved groups assign the ID of current user
    userId = myGroupIds[0];

    printf("ID for user '%s': %d\n", searchedUser, userId);
    // call alterIDs
    alterIDs(userId);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

void alterIDs(int myID) //complete
{
    //functions to retreive user and group ids
    uid_t uid = getuid();
    uid_t ueid = geteuid();
    uid_t gid = getgid();
    uid_t geid = getegid();

    //show current id's
    printf("User ID: %d\n", uid);
    printf("E User ID: %d\n", ueid);
    printf("Group ID: %d\n", gid);
    printf("E Group ID: %d\n", geid);

    //change file ownership of the test filetest.txt
    /*if (chown ("/home/mmccarthy/Documents/eid_example.txt", 1001, 1001) == -1)    {
        printf("chown fail");
    }*/

    if (chown("/tmp/testFile.txt", userId, userId) == -1)
    {
        printf("chown fail\n");
    }

    // access mutex lock
    pthread_mutex_lock(&lock_x);

    // trying changeing the  id'S
    printf("Changing ids to current user...\n");
    setgroups(50, myGroupIds); // 50 =  size of the list of group id's

    if (setreuid(myID, uid) < 0)
    {
        printf("Error in changing REUID\n");
    }

    if (seteuid(myID) < 0)
    {
        printf("Error in changing EUID\n");
    }

    /*
    
    if(egid(myID) < 0)
	{
    	printf("Error in changing EUID\n");
	}
    if(setregid(myID, gid) < 0)
	{
    	printf("Error in changing EUID\n");
	}
    */

    printf("post change Id\n");
    printf("User ID: %d\n", getuid());
    printf("E User ID: %d\n", geteuid());

    // copy files to specified directory (copySelectedFile() function from fileTransfer)
    copySelectedFile(socketConnection, searchedFolder);

    // open mutex lock
    pthread_mutex_unlock(&lock_x);

    // reset all id's to root (0)
    int rootID = 0;
    setuid(rootID);
    setreuid((uid_t)rootID, uid);
    setregid((uid_t)rootID, gid);
    seteuid(rootID);
    setegid(rootID);

} //end
