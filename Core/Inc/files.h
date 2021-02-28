/*
 * files.h
 *
 *  Created on: Dec 26, 2020
 *      Author: David
 */

#ifndef INC_FILES_H_
#define INC_FILES_H_
#include "main.h"

typedef enum{
	drive_nodrive,						// No drive on system
	drive_inserted,						// Drive was inserted
	drive_mounted,						// Drive was successfully mounted
	drive_ready,						// Drive is mounted and has files
	drive_nofiles,						// Drive mounted but no files
	drive_removed,						// Drive removed while mounted
	drive_error,						// Drive error
	drive_unmounted,					// Drive unmounted
}fileStatus;
void handleFS(void);
uint8_t find_mp3 (char* path);
uint8_t scanFS(void);
int32_t freespace(void);
#endif /* INC_FILES_H_ */
