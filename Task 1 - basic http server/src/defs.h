/* defs.h describes defines only */

/* Server specific defs */
#define BUFFER_SIZE     65000
/* method/header name + http in case of start line ~ around 1900 chars allowed, 
 * so that header value/path can be of max 8192 char long */
#define MAX_LINE_LENGTH 10000  
#define QUEUE_LENGTH    100
#define MAX_WRITE_LENGTH 32767

/* Http version def */
#define HTTPSTART       "HTTP/1.1 "
#define HTTPTITLE       "HTTP/1.1\r\n"

/* Error codes defs */
#define TWZZ            "200 Ok\r\n"
#define THZT            "302 File moved under different location\r\n"
#define FOZZ            "400 Wrong request format\r\n"
#define FOZF            "404 File not found\r\n"
#define FIZZ            "500 Server fail\r\n"
#define FIZO            "501 Server's method not implemented\r\n"

/* Defs used in header writing */
#define SERVERHEADER    "Server: Jakub_Grondziowski's_server_version_1.0\r\n"
#define TYPEHEADER      "Content-Type: application/octet-stream\r\n"
#define LENGTHHEADER    "Content-Length: "
#define LOCATIONHEADER  "Location: "
#define CONNECTIONHEADER "Connection: close\r\n"

/* Defs used as dictionary for headers, states and methods */
#define HEADCONNECT     1
#define HEADTYPE        2
#define HEADLENGTH      3
#define HEADSERVER      4

#define START_LINE      1
#define HEADERS         2

#define METHODGET       1
#define METHODHEAD      2

/* Unknown value used by method and header dictionary */
#define UNKNOWN         -1