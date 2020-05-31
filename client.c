//pwajok
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

//definition of functions
void getUserRelatedID();
void getGroupIDs();
void transferFile(int SID);

int main(int argc, char *argv[])
{
    int SID; //SOCKET ID
    struct sockaddr_in server;
    char buffer[1000];
    char selectedFolder[2000];
    char USER[2000];
    ssize_t n;

    // get user that's currently signed in
    char *user = getenv("USER");
    strcpy(USER, user);

    //Create socket
    SID = socket(AF_INET, SOCK_STREAM, 0);
    if (SID == -1)
    {
        printf("Error creating socket");
        exit(1);
    }
    printf("Socket is created");

    // set sockaddr_in variables
    server.sin_port = htons(8082);                   // Port to connect on
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP
    server.sin_family = AF_INET;                     //IPV4 protocol

    //Connect to server
    if ((connect(SID, (struct sockaddr *)&server, sizeof(server)) < 0))
    {
        printf("connect failed. Error");
        return 1;
    }

    printf("Connected to server ok!!\n");

    //keep communicating with server
    while (1)
    {
        // Prompt user to select a folder to transfer the file to
        printf("Select folder to send the file\n");
        printf("Options: \n");
        printf("Marketing\nOffers\nPromotions\nRoot\nSales\n");
        scanf("%s", selectedFolder);

        //If user inputs invalid option (checks against all options)
        if (strcmp(selectedFolder, "Marketing") != 0 && strcmp(selectedFolder, "Offers") != 0 && strcmp(selectedFolder, "Promotions") != 0 && strcmp(selectedFolder, "Root") != 0 && strcmp(selectedFolder, "Sales") != 0)
        {
            printf("!!Invalid Option selected!!\n");
            printf("Please select a valid option!\n");
        }
        else
        {
            printf("Matching folder found!\n");
            break;
        }
    }

    strcat(selectedFolder, " ");
    strcat(USER, " ");

    /*concat the two strings together in order to send username as well as the
	selected folder to the server as a single message (will be split by the server)*/
    strcat(selectedFolder, USER);

    printf("Passing Path to server...\n");
    if (send(SID, selectedFolder, strlen(selectedFolder), 0) < 0)
    {
        printf("Send failed \n");
        return 1;
    }

    //Call function to transfer the file
    transferFile(SID);

    ///home/john/Desktop/myFile.txt
    char serverMessage[2000];
    //check if server sent a reply
    if (recv(SID, serverMessage, 2000, 0) < 0)
    {
        printf("IO error");
        //break;
    }
    printf("Server sent: %s \n", serverMessage);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

//finction to send client messages to the server
void transferFile(int SID)
{
    FILE *selectedFile;
    char buffer[512];
    //filePath used to hold file path of file to be transfered
    char filePath[1000];

    while (selectedFile == NULL)
    {
        printf("Please enter path to file for transfer:\n");
        scanf("%s", filePath);
        //read in specified file
        selectedFile = fopen(filePath, "r");
        //check if the file exists at specified path
        if (selectedFile == NULL)
        {
            printf("!!Error!!\n The file %s doesn't exist\n Enter new file: ", filePath);
        }
        else
        {
            //if the file exists
            printf("File located...\n");
            printf("Copying file: %s to Server... ", filePath);
            //clear the buffer using bzero
            bzero(buffer, 512);

            int blksize = 0;
            int i = 0;
            while ((blksize = fread(buffer, sizeof(char), 512, selectedFile)) > 0)
            {
                printf("Data Sent %d = %d\n", i, blksize);
                if (send(SID, buffer, blksize, 0) < 0)
                {
                    printf("!!Error!!/nTransfer failed!\n");
                    printf("!!Aborting!!");
                    exit(1);
                }
                //clear the buffer
                bzero(buffer, 512);
                i++;

            } // end while
            printf("File transfer complete!\n");
            fclose(selectedFile);
        }
    }
}
