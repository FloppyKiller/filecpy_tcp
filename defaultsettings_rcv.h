/* 
 * File:   defaultsettings_rcv.h
 * Author: Ricardo Candussi
 * mail:   ricardo@metaebene.at
 *
 * Created on 03. April 2016, 10:40
 */

#ifndef DEFAULTSETTINGS_RCV_H
#define DEFAULTSETTINGS_RCV_H

//char default_file_Path[] = "output.tmp";              //default path to file
char default_server_Port[] = "12345";                   //default Server Port: 12345
int buffer_size = 1024;                                 //default buffer size: 1024
                                                        //make sure buffer size has same size on client side

/* 
 *   ^                                               ^
 *  /!\  DO NOT CHANGE SETTINGS BEYOND THIS PIONT!  /!\
 * /___\      COULD CAUSE UNINTENDED BEHAVIOUR     /___\
 * 
 */
u_int32_t MB_devider = 1e+6;                            //1 Byte = 8 Bit
                                                        //1 Kilobyte = 1,024 Bytes
                                                        //1 Megabyte = 1,048,576 Bytes
char default_protocol[] = "tcp";                        //used protocol std=tcp
                                                        //currently only tcp is supported depending on socket

#endif /* DEFAULTSETTINGS_RCV_H */
