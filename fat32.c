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

int16_t NextLB(uint32_t sector)
{
	uint32_t FATAddress = (BPB_BytesPerSec *BPB_RsvdSecCnt)+(sector * 4);
	int16_t val;
	fseek(fp, FATAddress, SEEK_SET);
	fread(&val, 2, 1, fp);
	return val;
}

int LBAToOffset(int32_t sector)
{
	return ((sector - 2)*BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt) + (BPB_NumFATS * BPB_FATSz32*BPB_BytesPerSec);
}
/*void get(char *filename)
{
	int found = 0;
	int i = 0;
	
	for(i = 0; i <11; i++)
	{
		if(compare(i, filename))
		{
			found = i;
			break;
		}
	}
	if(found == 0)
	{
		printf("File not found\n");
	}
	
	int cluster = dir[i].FirstClusterLow);
	int size = dir[i].FileSize;
	int offset = LBAToOffset(cluster)fseek(fp, offset,SEEK_SET);
	ofp.fopen(filename,"r);
	char buff[512];
	fread(&buff[0],512,',fp);
	fwrite(&buff[0],512,ofp);
	size = size-512;
	
	strcpy(string, dir[i].DIR_Name);
	string[11] = '\0';
}
*/



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


//int stat(const char *path, struct stat *___attribute__);

int main(void)
{
	char *cmd_str = (char*)malloc(MAX_COMMAND_SIZE);
	//sig_int();
	 
	while(1)
	{
		printf("msf> "); 
		while(!fgets(cmd_str, MAX_COMMAND_SIZE, stdin) );
		

	char *token[MAX_NUM_ARGUMENTS];
	int token_count = 0;
	char *arg_ptr;
	char *working_str = strdup(cmd_str);
	char *working_root = working_str;
	  
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
		
		if(strcmp(token[0],"put") == 0)
		{
			if(fp != NULL)
			{
			}
			if(fp == NULL)
			{
				printf("Error: File system must be opened first.\n");
			}
		}
		if(strcmp(token[0],"cd") == 0)
		{
			if(fp != NULL)
			{
			}
			if(fp == NULL)
			{
				printf("Error: File system must be opened first.\n");
			}
		}
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
					
			if(fp == NULL)
			{
				printf("Error: File system must be opened first.\n");
			}
		}
	}
	
	
 return EXIT_SUCCESS;
 