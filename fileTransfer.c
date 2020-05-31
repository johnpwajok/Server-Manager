//pwajok
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <grp.h>
#include <pwd.h>
#include "mainheader.h"

//path for serverFiles directory
char selectedFile[500] = "/home/john/Desktop/serverFiles/serverDir/";

void copySelectedFile(int socketConnection, char *searchedFolder)
{
    /* concat the selected folder (folderToSearchFor) to the main directory path (/Desktop/serverFiles)
	for the full path*/
    strcat(selectedFile, searchedFolder);
    printf("Copying file to : %s ...\n", selectedFile);

    //copyTest used to check whether the copy operation works
    int copyTest;
    char copyCommand[500] = "cp /tmp/test.txt ";
    strcat(copyCommand, selectedFile);
    int copyCmd = system(copyCommand);

    if (copyCmd == -1)
    {
        printf("!!ERROR!! \t 'copy' command failed\n");
    }


    // remove the folder word from the string above
    // so that the next folder will get appended 
    int i, counter = 0, flag = 0, k = 0, n = 0;

    for (i = 0; selectedFile[i] != '\0'; i++)
    {
        k = i;

        while (selectedFile[i] == searchedFolder[counter])
        {
            i++, counter++;
            if (counter == strlen(searchedFolder))
            {
                flag = 1;
                break;
            }
        }
        counter = 0;

        if (flag == 0)
            i = k;
        else
            flag = 0;

        selectedFile[n++] = selectedFile[i];
    }

    selectedFile[n] = '\0';

    printf("File after word has been removed : %s\n", selectedFile);
}


