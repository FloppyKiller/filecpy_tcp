/* 
 * File:   defaultsettings.h
 * Author: Ricardo Candussi
 * mail:   ricardo@metaebene.at
 *
 * Created on 03. April 2016, 04:24
 */

#ifndef DEFAULTSETTINGS_SND_H
#define DEFAULTSETTINGS_SND_H

#if defined (__APPLE__) || defined (__MACH__)           //check if target OS is APPLE (OS X)

#define __S_IFDIR	S_IFDIR				//OS X defines '__S_IFDIR' as 'S_IFDIR'
                                                        //other unix_like dist. systems seems to be fine with '__S_IFDIR'

#endif

//char default_file_Path[] = "input.tmp";               //default path to file
char default_server_Port[] = "12345";                   //default Server Port: 12345
char default_server_IP[] = "127.0.0.1";                 //default Server IP: 127.0.0.1 (localhost))
int buffer_size = 1024;                                 //default buffer size: 1024
                                                        //make sure buffer size has same size on server side

/* 
 *   ^                                               ^
 *  /!\  DO NOT CHANGE SETTINGS BEYOND THIS PIONT!  /!\
 * /___\      COULD CAUSE UNINTENDED BEHAVIOUR     /___\
 * 
 */
u_int32_t MB_devider = 1e+6;                            //1 Byte = 8 Bit
u_int32_t max_file_size = 4e+9;                        	//1 Kilobyte = 1,024 Bytes
                                                        //1 Megabyte = 1,048,576 Bytes
char default_protocol[] = "tcp";                        //used protocol std=tcp
                                                        //currently only tcp is supported depending on socket

#endif /* DEFAULTSETTINGS_SND_H */
