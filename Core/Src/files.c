/*
 * files.c
 *
 *  Created on: Dec 26, 2020
 *      Author: David
 */


#include "files.h"
#include "fatfs.h"
FATFS fatfs;
char* paths[] = { "/CD01","/CD02","/CD03","/CD04","/CD05","/CD06" };

void handleFS(void){
	FRESULT res;
	if(systemStatus.status==drive_inserted){						// Drive present
		if( f_mount(&fatfs, "", 1) != FR_OK){
			  printf("Failed to mount volume\n\n");
			  systemStatus.status=drive_error;						//Failure on mount
		}
		else{
			  printf("Volume mounted\n");
			  systemStatus.status=drive_mounted;					// Mounted
			  int32_t size = freespace();
			  if(size==-1){
				  systemStatus.status=drive_error;
			  }
			  else{
				  if(size!=systemStatus.lastFsSize){
					  systemStatus.lastFsSize=size;
					  systemStatus.lastFile[0]=0;					// Different drive or changed contents, reset stored data
					  systemStatus.lastPath[0]=0;
					  printf("Different drive\n");
				  }
				  else{
					  printf("Same drive\n");						// Keep stored data
				  }
				  res=scanFS();										// Perform file scan and open last file if any
				  if(res!=FR_OK){
					  systemStatus.status=drive_error;				// No files or drive error
				  }
				  else{
					  systemStatus.status=drive_ready;				// All ok
				  }
			  }

		}
		if(systemStatus.status==drive_ready){						// If all ok
			mag_data.cmd2=0;
			for(uint8_t i=0;i<6;i++){								// Transfer found files to cd info for unilink
				if(systemStatus.fileCnt[i]){
					cd_data[i].tracks=systemStatus.fileCnt[i];
					cd_data[i].mins=80;
					cd_data[i].secs=0;
					switch(i){									// Set mag data disc presence
						case 0:
							mag_data.CD1=1;
							break;
						case 1:
							mag_data.CD2=1;
							break;
						case 2:
							mag_data.CD3=1;
							break;
						case 3:
							mag_data.CD4=1;
							break;
						case 4:
							mag_data.CD5=1;
							break;
						case 5:
							mag_data.CD6=1;
							break;
					}
				}
				else{
					cd_data[i].tracks=99;
					cd_data[i].mins=0;
					cd_data[i].secs=0;
				}
			}
			unilink.mag_changed=1;									// set flag to update
		}
	}
	if((systemStatus.status==drive_error)||(systemStatus.status==drive_removed)){	//if drive removed or error
		f_mount(0, "", 1);															// remove mount point
		if(systemStatus.status==drive_removed){
			systemStatus.status=drive_nodrive;
		}
		else{
			systemStatus.status=drive_unmounted;
		}
		mag_data.cmd2=0;
		mag_data.CD1=1;																// Set only cd 1 ("aux" mode)
		cd_data[0].tracks=1;
		cd_data[0].mins=80;
		cd_data[0].secs=0;
		for(uint8_t i=1;i<6;i++){
		  cd_data[i].tracks=99;
		  cd_data[i].mins=0;
		  cd_data[i].secs=0;
		}
		unilink.mag_changed=1;														// set flag to update
	}
}

int32_t freespace(void){
	 DWORD fre_clust, fre_sect; //tot_sect;
	 FRESULT res;
	 FATFS* fatfs2=&fatfs;
	 /* Get volume information and free clusters of drive 1 */
	 res = f_getfree("", &fre_clust, &fatfs2);
	 if(res){
		 return -1;
	 }

	 /* Get total sectors and free sectors */
	// tot_sect = (fatfs.n_fatent - 2) * fatfs.csize;
	 fre_sect = fre_clust * fatfs.csize;

	 /* Print the free space (assuming 512 bytes/sector) */
	 //printf("%10lu KiB total drive space.\n%10lu KiB available.\n", tot_sect / 2, fre_sect / 2);
	 return(fre_sect);
}

uint8_t scan_files(char* path){        /* Start node to be scanned (***also used as work area***) */
	FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

uint8_t find_mp3 (char* path){

    FRESULT res;     /* Return value */
    DIR dir;         /* Directory object */
    FILINFO fil;    /* File information */
    uint8_t c=0;
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
		res = f_findfirst(&dir, &fil, path, "*.MP3");  	  /* Start to search for mp3 files */
		while (res == FR_OK && fil.fname[0]) {         /* Repeat while an item is found */
			c++;
			//printf("%s\n", fno.fname);                /* Print the object name */
			res = f_findnext(&dir, &fil);               /* Search for next item */
		}
    }
    f_closedir(&dir);
    return c;
}
uint8_t restorelast(void){
	#define SZ_TBL	8
	FRESULT res;     /* Return value */
	DIR dp;         /* Directory object */
	FILINFO fno;    /* File information */
	FIL fp;    	/* File object */
	DWORD clmt[SZ_TBL];

	//strcpy(systemStatus.lastPath,"/CD01");
	//strcpy(systemStatus.lastFile,"1.TXT");

	if((systemStatus.lastPath[0]!=0)&&(systemStatus.lastFile[0]!=0)){		// If stored path and file
		printf("Stored path: \"%s\"\n",systemStatus.lastPath);
		printf("Stored file: \"%s\"\n",systemStatus.lastFile);
	}
	else if( (systemStatus.lastFile[0]==0)||(systemStatus.lastPath[0]==0)){	// If any not stored, reset stored data
		systemStatus.lastFile[0]=0;
		systemStatus.lastPath[0]=0;
		printf("No previous stored file\n");
		uint8_t i=0;
		while(i<6){
			if(systemStatus.fileCnt[i]){									// search first CD folder with files
				strcpy(systemStatus.lastPath,paths[i]);						// store folder path
				printf("Found %3d files in \"%s\"\n",systemStatus.fileCnt[i],paths[i]);
				break;
			}
			i++;
		}
		if(systemStatus.lastPath[0]==0){
			printf("No files found in filesystem");						// No compatible files in the filesystem
			return FR_DISK_ERR;
		}
	}
	res = f_chdir(systemStatus.lastPath);								// Change dir
	if(res){
		printf("Error opening folder \"%s\"\n",systemStatus.lastPath);
		return FR_DISK_ERR;
	}
	if(systemStatus.lastFile[0]!=0){
		res = f_open(&fp, systemStatus.lastFile, FA_READ);				// Open stored file
		if(res==FR_OK){

			printf("Opened File: \"%s\"\n",systemStatus.lastFile);
			fp.cltbl = clmt;                      /* Enable fast seek mode (cltbl != NULL) */
			 clmt[0] = SZ_TBL;                      /* Set table size */
			 res = f_lseek(&fp, CREATE_LINKMAP);
			 if(res){
				 printf("Error creating linkmap\n");
				 return FR_DISK_ERR;
			 }
			 printf("Linkmap file created\n");
/*
			UINT count;
			uint32_t t=HAL_GetTick();
			#define ts	512
			__attribute__((aligned(4)))  char data[ts];
			uint32_t total=0;
			//FSIZE_t size;
			//size=f_size(&fp);
			//return;


			do{							// Read whole file

				//HAL_DMA_Start(&hdma_memtomem_dma2_stream0,(uint32_t)rbuff + ((fp->sect - sect) * SS(fs)),(uint32_t)&fp->buf,SS(fs));
				//HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream0, HAL_DMA_FULL_TRANSFER, 3000);
				res=f_read(&fp, &data[0], ts, &count);
				if(res!=FR_OK){
					asm("nop");
					return FR_DISK_ERR;
				}
				if(count){
					total+=count;
					if(count<ts){
						break;
					}
					//printf(data);
				}
			}while(count);
			printf("%ldKB en %ldmS, %ldKB/s\n",total/1024,HAL_GetTick()-t,total/(HAL_GetTick()-t));
			*/
			return FR_OK;												// OK, done here
		}
		else{
			printf("Error opening \"%s\"\n",systemStatus.lastFile);		// Error reading stored filenamew
		}
	}
	res = f_findfirst(&dp, &fno, systemStatus.lastPath, "*.MP3");  	  	// Search first mp3 file
	if(res!=FR_OK){
		printf("Error: No files found\n");								// No files found
		return FR_DISK_ERR;												// We should have files from previous scan
	}
	strcpy(systemStatus.lastFile,fno.fname);							// OK, store filename
	printf("Found  File: \"%s\"\n",fno.fname);/*Opened*/
	res = f_open(&fp, systemStatus.lastFile, FA_READ);					// Open stored file
	if(res==FR_OK){
		printf("Opened File: \"%s\"\n",systemStatus.lastFile);
		return FR_OK;
	}
	printf("Error opening \"%s\"\n",systemStatus.lastFile);		// Error
	return FR_DISK_ERR;
}

uint8_t scanFS(void){
    FRESULT res;
    uint8_t c,i=0;
    /*
    printf("Trying /:\n");

	res = scan_files(buff);
*/

    /*
    strcpy(buff, "/");
    printf("Estructura de archivos en %s:\n",buff);
    scan_files(buff);
    printf("\n\n");
    */
    systemStatus.status=drive_nofiles;
    while(i<6){
    	c=find_mp3(paths[i]);
    	systemStatus.fileCnt[i]=c;
		if(c){
			systemStatus.status=drive_ready;					// If any folder with mp3 files is found
			printf("%s:%3d MP3 files\n",paths[i],c);
		}
		i++;
    }
    if(systemStatus.status==drive_ready){						// filesystem mounted, scanned and mp3 found
    	res= restorelast();											// Restore last file if any
    	if(res==FR_OK){
    		printf("Done\n");
    		return FR_OK;
    	}
    }
	printf("No files found\n");
	return FR_DISK_ERR;
}
