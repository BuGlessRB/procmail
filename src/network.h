/*$Id: network.h,v 1.2 1992/10/02 14:40:47 berg Exp $*/

#include <sys/socket.h>			/* socket() sendto() AF_INET
					   SOCK_DGRAM */
#include <netdb.h>			/* gethostbyname() getservbyname()
					   getprotobyname() */
#include <netinet/in.h>			/* htons() struct sockaddr_in */

#ifndef BIFF_serviceport
#define BIFF_serviceport	COMSATservice
#endif
