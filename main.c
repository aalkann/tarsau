#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <unistd.h>
#include <dirent.h>

int getFileSize(char* fileName){
    FILE *file = fopen(fileName,"r");
    fseek(file,0,SEEK_END);
    int size = ftell(file);
    fclose(file);
    return size;
}
int isDirectoryExists(const char *path) {
    struct stat info;
    if (stat(path, &info) != 0) {
        return 0;  // Directory does not exist
    }
    return (info.st_mode & __S_IFDIR) != 0;
}

int createDirectory(const char *path) {
    if(path== NULL) return 0;
    int status = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return status;
}

int getFilePermissions(const char *filename) {
    struct stat fileStat;
    if (stat(filename, &fileStat) == -1) {
        perror("Error in getting file information");
        return -1;  
    }
    int decimalNumber = fileStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    return decimalNumber;
}

mode_t getFilePermissionAsMode(const char *filename) {
    struct stat fileStat;
    if (stat(filename, &fileStat) == -1) {
        perror("Error in getting file information");
        return -1;  
    }
    return fileStat.st_mode ;
}

char *getPermissionString(const char *filename) {
    struct stat fileStat;
    if (stat(filename, &fileStat) == -1) {
        perror("Error in getting file information");
        exit(1);  
    }
    mode_t mode = fileStat.st_mode;
    static char perms[10];

    // User permissions
    perms[0] = (mode & S_IRUSR) ? 'r' : '-';
    perms[1] = (mode & S_IWUSR) ? 'w' : '-';
    perms[2] = (mode & S_IXUSR) ? 'x' : '-';

    // Group permissions
    perms[3] = (mode & S_IRGRP) ? 'r' : '-';
    perms[4] = (mode & S_IWGRP) ? 'w' : '-';
    perms[5] = (mode & S_IXGRP) ? 'x' : '-';

    // Other permissions
    perms[6] = (mode & S_IROTH) ? 'r' : '-';
    perms[7] = (mode & S_IWOTH) ? 'w' : '-';
    perms[8] = (mode & S_IXOTH) ? 'x' : '-';

    perms[9] = '\0'; // Null-terminate the string

    return perms;
}

int getDecimalLength(int number) {
    int length = 0;
    do {
        length++;
        number /= 10;
    } while (number != 0);

    return length;
}

int writeOrganization( char* fileNames[],int fileCounts,FILE* file){

    for (int i = 0; i < fileCounts; i++)
    {
        char * fileName = fileNames[i];
        char* perms = getPermissionString(fileName);
        int permsN = getFilePermissions(fileName);
        fprintf(file,"| %s , %s(%o) , %d |\n",fileName,perms,permsN,getFileSize(fileName));

    }
    int recordsSize = ftell(file);
    int recordSizeLength = getDecimalLength(recordsSize);
    return recordsSize + recordSizeLength;
}
void writeOrganizationSize(char* fileNames[],int fileCounts,FILE* file,int size){
      
    fprintf(file,"%d\n",size);
    for (int i = 0; i < fileCounts; i++)
    {
        char * fileName = fileNames[i];
        char* perms = getPermissionString(fileName);
        int permsN = getFilePermissions(fileName);
        fprintf(file,"| %s , %s(%o) , %d |\n",fileName,perms,permsN,getFileSize(fileName));

    }
}

// GLOBAL VARIABLES
int MAXFILESIZE = 209715200; // 200 Mbyte
int MAXINPUTFILECOUNT = 32;
//

int main(int argc, char* argv[]){

    
    if(argc <= 2){ // NO Argument 
        printf("Missing Argument\n");
        exit(1);
    }
    else{
        if(!strchr(argv[1],'-')){ // Wrong Order
            printf("Wrong Order");
            exit(1);
            
        }
    }

    char mainOperator = argv[1][1];
    char currentOperator;
    // Variables for -b
    char* inputFileNames[argc-2];
    int inputFileCount = 0;
    // Variables for -o
    char* outputFileName="a.sau";
    // Variables for -a
    char* archiveFileName=NULL;
    char* archiveOpenDirectoryName=NULL;

    for (int i = 1; i < argc; i++)
    {
        if(strchr(argv[i],'-')){
            currentOperator = argv[i][1];
            continue;
        } 

        if(inputFileCount == MAXINPUTFILECOUNT){
            printf("Number of input file cannot be more than %d \n",MAXINPUTFILECOUNT);
            exit(1);
        }

        switch (currentOperator)
        {
        case 'b':
            if(access(argv[i],F_OK) == -1){
                printf("File '%s' does not exist\n",argv[i]);
                exit(1);
            }
            inputFileNames[inputFileCount] = argv[i];
            inputFileCount++;
            break;
        case 'o':
            outputFileName = argv[i];
            break;
        case 'a':
            if(argc == 3){
                archiveFileName = argv[2];
            }
            else if(argc == 4){
                archiveFileName = argv[2];
                archiveOpenDirectoryName = argv[3];
            }
            else{
                perror("Wrong number of parameter");
                exit(1);
            }
        default:
            break;
        }

    }
    

    switch (mainOperator)
    {
    case 'b':
        // Control the total size of input files
        int totalFileSize = 0;
        for (int i = 0; i < inputFileCount; i++)
        {
            totalFileSize += getFileSize(inputFileNames[i]);
        }
        
        if(totalFileSize> MAXFILESIZE){
            perror("You exceeded the maximum file size");
            exit(1);
        }
        //


        FILE* outputFile = fopen(outputFileName,"w");
        if (outputFile == NULL){
            perror("Error opening output file");
            exit(1);
        }


        int headerSize = writeOrganization(inputFileNames,inputFileCount,outputFile);
        fseek(outputFile,0,SEEK_SET);
        writeOrganizationSize(inputFileNames,inputFileCount,outputFile,headerSize);

        for (int i = 0; i < inputFileCount; i++)
        {
            FILE* inputFile = fopen(inputFileNames[i],"r");
            if (inputFile == NULL) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }

            int c;
            while ((c = fgetc(inputFile)) != EOF) {
                fputc(c, outputFile);
            }
            fclose(inputFile);
        }
        printf("Total Input File Size = %d \n",totalFileSize);

        fclose(outputFile);
        printf("Files combined successfully into %s.\n", outputFileName);
        break;
    
    case 'a':

        FILE* archiveFile = fopen(archiveFileName,"r");
        if (archiveFile == NULL){
            perror("Archive file is inappropriate or corrupt!");
            exit(1);
        }

        if(!isDirectoryExists(archiveOpenDirectoryName))
            createDirectory(archiveOpenDirectoryName);
        
        char buffer[1024];
        char* word = (char*) malloc(100);
        char* fileName = (char*) malloc(100);
        char * filePerm = (char*) malloc(100);
        char* filePermDecimal = (char*) malloc(30);
        int fileSize;
        char* path = (char* )malloc(300);
        
        fgets(buffer,sizeof(buffer),archiveFile);

        int cursorRecordLeft = ftell(archiveFile);
        int cursorReadStart = atoi(buffer);

        while (fgets(buffer,sizeof(buffer),archiveFile)!= NULL)
        {  
            if(buffer[0] != '|') break;
            word = strtok (buffer,",|()");
            strcpy(fileName,word);
            
            word = strtok (NULL, ",|()");
            word = strtok (NULL, ",|()");
            strcpy(filePermDecimal,word);

            word = strtok (NULL, ",|()");
            word = strtok (NULL, ",|()");
            fileSize = atoi(word);

            cursorRecordLeft = ftell(archiveFile);
            fseek(archiveFile,cursorReadStart+1,SEEK_SET);

            FILE* newFile = NULL;
            if(archiveOpenDirectoryName == NULL){
                newFile = fopen(fileName,"w");
                strcpy(path,fileName);
            }
            else{
                path = strcpy(path,archiveOpenDirectoryName);
                path = strcat(path,"/");
                newFile = fopen(strcat(path,fileName),"w");
            }

            if (newFile == NULL) {
                printf("Error opening the file.\n");
                return 1;
            }

            int c;
            int byteCounter= 0;
            while (c = fgetc(archiveFile))
            {   
                fputc(c,newFile);
                byteCounter++;
                if(byteCounter== fileSize) break;
            }

            cursorReadStart += fileSize;
            fclose(newFile);

            // Change file Permission
                mode_t filePerm = (mode_t) strtol(filePermDecimal, NULL, 8);
                // Set the permissions of the new file to be the same as the existing file

                if (chmod(path, filePerm) == -1) {
                    perror("Error setting new file permissions");
                    return EXIT_FAILURE;
                }
            //


            fseek(archiveFile,cursorRecordLeft,SEEK_SET);
        }

        free(path);
        free(fileName);
        free(filePermDecimal);
        free(filePerm);
        fclose(archiveFile);
        break;
        
    default:
        break;
    }

    return 1;
}