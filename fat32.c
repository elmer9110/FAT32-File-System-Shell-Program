/*
	Name: Elmer Rivera
	ID: 1001529110
*/
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <readline/history.h>
#include <errno.h>
#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENTS 10

//Initialization of all variables needed
//for fat32 file information and file handling
int i;
int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATS;
int32_t BPB_FATSz32;
int16_t BPB_RootEntCnt;
char BS_VolLab[11];
int32_t BPB_RootClus;
int32_t RootAddress;

int32_t RootDirSectors = 0;
int32_t FirstDataSector = 0;
int32_t FirstSectorofCluster = 0;


FILE *ofp;
FILE *fp; 

//Function that stores the information of 
//the fat32 file system by seeking to the bytes
//where the information  is stored and then assigned
//to variables so they can be displayed when "info" is
//called
void Fat32Info()
{
	//BytesPerSec
	fseek(fp,11,SEEK_SET);
	fread(&BPB_BytesPerSec,1,2,fp);
	//SrcPerClus
	fseek(fp,13,SEEK_SET);
	fread(&BPB_SecPerClus,1,1,fp);
	//RsvdSecCnt
	fseek(fp, 14, SEEK_SET);
	fread(&BPB_RsvdSecCnt,1,2,fp);
	//BPB_NumFATS
	fseek(fp, 16, SEEK_SET);
	fread(&BPB_NumFATS,1,1,fp);
	//BPB_FATSz32
	fseek(fp, 36, SEEK_SET);
	fread(&BPB_FATSz32,1,4,fp);
			
}

//Function used to compare a filename given by the user
//to the filenames in the fat32.
void expand(char *filename, char *expanded)
{
	char expanded_name[12];
	memset( expanded_name, ' ', 12 );
	
	char *token2 = strtok( filename, "." );
	strncpy( expanded_name, token2, strlen( token2 ) );
	token2 = strtok( NULL, "." );
	if( token2 )
	{
		strncpy( (char*)(expanded_name+8), token2, strlen(token2 ) );
	}
	int i;
	for( i = 0; i < 11; i++ )
	{
		expanded_name[i] = toupper( expanded_name[i] );
	}  
	strncpy(expanded,expanded_name,strlen(expanded_name));
}

//Function used to find the next logical block address in the first FAT 
//of the logical block given by the user
int16_t NextLB(uint32_t sector)
{
	uint32_t FATAddress = (BPB_BytesPerSec *BPB_RsvdSecCnt)+(sector * 4);
	int16_t val;
	fseek(fp, FATAddress, SEEK_SET);
	fread(&val, 2, 1, fp);
	return val;
}

//Function that takes in a sector number that points to a block of data
//and returns the starting address for the given block of data
int LBAToOffset(int32_t sector)
{
	return ((sector - 2)*BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt) + (BPB_NumFATS * BPB_FATSz32*BPB_BytesPerSec);
}



//Structure initialization that will store the several
//variables associated with the files inside the fat32
//such as their size, cluster number, name,etc.
struct __attribute__((__packed__)) DirectoryEntry{
char DIR_Name[11];
uint8_t DIR_Attr;
uint8_t Unused1[8];
uint16_t DIR_FirstClusterHigh;
uint8_t Unused2[4];
uint16_t DIR_FirstClusterLow;
uint32_t DIR_FileSize;
};
struct DirectoryEntry dir[16];



int main(void)
{
	char *cmd_str = (char*)malloc(MAX_COMMAND_SIZE);
	
	 
	while(1)
	{
        //print out the shell command prompt
		printf("mfs> "); 

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
		while(!fgets(cmd_str, MAX_COMMAND_SIZE, stdin) );
		

	    char *token[MAX_NUM_ARGUMENTS];
	    int token_count = 0;
	    char *arg_ptr;
	    char *working_str = strdup(cmd_str);
	    char *working_root = working_str;
	  
        // Tokenize the input stringswith whitespace used as the delimiter
		while( ( ( arg_ptr = strsep(&working_str, WHITESPACE) ) != NULL) && (token_count < MAX_NUM_ARGUMENTS) )
		{
			token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
		 
			if(strlen(token[token_count]) == 0)
			{
				token[token_count] == NULL;
			}
		     token_count++;
		
		}

	    int fileOpen;

        //Check if the "open" command was given in order
        //to open the fat32.img file system and initialize
        //the root directory address so we can seek to it and
        //populate our directory array with the directories in the 
        //FAT
		if(strcmp(token[0], "open") == 0)
		{
			if (fp == NULL)
			{
				fp = fopen(token[1],"r");

				if( fp == NULL) 
				{
					printf("Error: File system image not found.\n");
				}
			}
			else if (fp != NULL)
			{
				printf( "Error: File system image already open.\n"); 
			}	
			Fat32Info();

	        // Calculating address of root directory
	        RootAddress = (BPB_NumFATS * BPB_FATSz32 * BPB_BytesPerSec) + (BPB_RsvdSecCnt * BPB_BytesPerSec);
	        fseek(fp, RootAddress, SEEK_SET);
	  
	        int i;
	        // Reads through the 16 directory entries
	        for(i = 0; i < 16; i++)
	        {
		        fread(&dir[i], sizeof(struct DirectoryEntry), 1, fp);
	        }
		}

        //Check to see if the "Close" command was given
        //so we can close the fat32.img in our shell
		if(strcmp(token[0],"close") == 0)
		{
			if (fp != NULL)
            {
				fclose(fp);
			}
			else
			{
				printf("Error: File system not open\n");
            }
			fp = NULL;
		}
			
		//Check to see if the "info" command was given
        //and then display the fat32.img specs in both decimal
        //and hexadecimal
		if(strcmp(token[0],"info") == 0 )
		{
			if(fp != NULL)
			{
			    printf("BPB_BytesPerSec: %d %x ", BPB_BytesPerSec, BPB_BytesPerSec);
			    printf("\n");
			    printf("BPB_SecPerClus: %d %x",BPB_SecPerClus, BPB_SecPerClus);
			    printf("\n");
			    printf("BPB_RsvdSecCnt: %d %x ", BPB_RsvdSecCnt, BPB_RsvdSecCnt); 
			    printf("\n");
			    printf("BPB_NumFATS: %d %x ", BPB_NumFATS, BPB_NumFATS);
			    printf("\n");
			    printf("BPB_FATSz32: %d %x ", BPB_FATSz32, BPB_FATSz32);
			    printf("\n");
		    }
			if(fp == NULL)
			{
			    printf("Error: File system must be opened first.\n");
		    }
		}
			
		//Check to see if the "stat" command was given
        //and then take the specified filename specified by the user
        //,verify it using the expand function, and then print its
        //attribute, filesize, and cluster number.
		if(strcmp(token[0],"stat") == 0)
		{	if(fp != NULL)
			{
				
			char expanded_name[12]; 
			expand(token[1], &expanded_name[0]);
			expanded_name[11] = '\0';
            for(i = 0; i < 16; i++)
			{
				int attr;
				int first_char;
				char name[12];
				memcpy(name, dir[i].DIR_Name,12);
				name[11] = '\0';
				attr = dir[i].DIR_Attr;
				first_char = dir[i].DIR_Name[0];
					
				if(((attr == 0x01)||(attr == 0x10)||(attr == 0x20))&&((first_char != 0x00) && (first_char!= 0xE5)))
				{
					if(strcmp(name, expanded_name) == 0)
					{
					    printf("Attribute \t Size \t Starting Cluster Number \n");
						printf("%d \t\t %d \t\t %d \n",dir[i].DIR_Attr,dir[i].DIR_FileSize,dir[i].DIR_FirstClusterLow);
					}     
				}
			}
							
			}
			if(fp == NULL)
			{
				printf("Error: File system must be opened first.\n");
			}
		}
		
        //Check to see if the "get" command was given
        //and then take the specified file and move it into
        // the current working directory of the user.
        //*****NOT IMPLEMENTED CURRENTLY
		if(strcmp(token[0],"get") == 0)
		{
			if(fp != NULL)
			{
			}
			if(fp == NULL)
			{
				printf("Error: File system must be opened first.\n");
			}
		}
		
        //Check to see if the "cd" command was given
        //and establish the path to the directory the user
        //specified in order to make it the current working 
        //directory
		if(strcmp(token[0],"cd") == 0)
		{
			if(fp != NULL)
			{
                int addresses[300];
                char expanded_name[12]; 
                int count=0;
                if(strcmp(token[1],"..")==0)
                {
                    int parentoffs;
                    
                    int cluster=dir[1].DIR_FirstClusterLow;
                    if(cluster==0)
                    {
                        parentoffs=RootAddress;
                        fseek(fp,parentoffs,SEEK_SET);
                        fread(&dir[0],16, sizeof(struct DirectoryEntry), fp);
                    }
                    else
                    {
                        parentoffs=LBAToOffset(cluster);
                        fseek(fp,parentoffs,SEEK_SET);
                        fread(&dir[0],16, sizeof(struct DirectoryEntry), fp);
                    }
                    
                    
                }
                else
                {
				    for(i = 0; i < 16; i++)
				    {
                        int attr;
					    int first_char;
					    char name[12];
                        int offs;
                        expand(token[1], &expanded_name[0]);
			            expanded_name[11] = '\0';
					    memcpy(name, dir[i].DIR_Name,12);
					    name[11] = '\0';
					    attr = dir[i].DIR_Attr;
					    first_char = dir[i].DIR_Name[0];
                    
					    if(((attr == 0x01)||(attr == 0x10)||(attr == 0x20))&&((first_char != 0x00) && (first_char!= 0xE5)))
					    {
						    if(strcmp(name, expanded_name) == 0)
						    {
                                offs=LBAToOffset(dir[i].DIR_FirstClusterLow);
                                addresses[count]=offs;
                                count++;
                                fseek(fp,offs,SEEK_SET);
                                fread(&dir[0],16, sizeof(struct DirectoryEntry), fp);
						    }
					    }
				    }
                }
			}
			if(fp == NULL)
			{
				printf("Error: File system must be opened first.\n");
			}
		}
        
        //Check to see if the "ls" command was given
        //in order to display all the files/directories in the current directory
        //the user is in. Only files and directories that have not been deleted
        //and are read only, and are subdirectories are shown
		if(strcmp(token[0],"ls") == 0)
		{
			if(fp != NULL)
			{
				int attr;
				int first_char;
				char name[12];
				for(i = 0; i < 16; i++)
				{
					strncpy(name, dir[i].DIR_Name,12);
					name[11] = '\0';
					attr = dir[i].DIR_Attr;
					first_char = dir[i].DIR_Name[0];
					if(((attr == 0x01)||(attr == 0x10)||(attr == 0x20))&&((first_char != 0x00) && (first_char!= 0xE5)))
						{
							
							printf("%s \n",name);
							
						}
				}
			}
			if(fp == NULL)
			{
				printf("Error: File system must be opened first.\n");
			}
		}

        //Check to see if the "read" command was given
        //and reads the user given file at the position, in bytes, specified by 
        //the position parameter and outputs the number of bytes specified. 
		if(strcmp(token[0],"read") == 0)
		{
			if(fp != NULL)
			{
				if((token[2] != NULL) && (token[3] != NULL))
				{
					char expanded_name[12];
					expand(token[1],&expanded_name[0]);
					expanded_name[11] = '\0';
					
					int position = atoi(token[2]);
					int numbytes = atoi(token[3]);
					
					int LowCluster = -1;
					int filesize = -1;
					char name[12];
					
					for(i = 0; i <16; i++)
					{
						strncpy(name, dir[i].DIR_Name,12);
						name[11] = '\0';
						if(strcmp(name, expanded_name) == 0)
						{
							LowCluster = dir[i].DIR_FirstClusterLow;
							filesize = filesize - 512;
						}
					}
					
					char buff[512];
					int offset = LBAToOffset(LowCluster)+position;
					int cluster = LowCluster;
					strncpy(name, token[1],12);
					
					while(filesize > 512)
					{
						fseek(fp, offset, SEEK_SET);
						fread(&buff[0],1,512,fp);
						int j;
						for(j = 0; j<512;j++)
						{
							printf("%x ", buff[j]);
						}
						filesize = filesize - 512;
						cluster = NextLB(cluster);
						offset = LBAToOffset(cluster);
					}
					if(filesize > 0)
					{
						fseek(fp, offset, SEEK_SET);
						fread(&buff[0],filesize,1,fp);
						
						int j;
						for(j = 0; j < filesize;j++)
						{
							printf("%x ",buff[j]);
						}
					}
				}
			}

            //Check that executes whenever the user attempts to 
            //enter commands and the fat32.img file has not been
            //opened yet.	
			if(fp == NULL)
			{
				printf("Error: File system must be opened first.\n");
			}
		}
	}
	
	
 return EXIT_SUCCESS;

}
