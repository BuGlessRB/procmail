/*$Id: network.h,v 1.6 1994/05/26 14:13:17 berg Exp $*/

#include <sys/socket.h>			/* socket() sendto() AF_INET
					/* SOCK_DGRAM */
#include <netdb.h>			/* gethostbyname() getservbyname()
					/* getprotobyname() */
#include <netinet/in.h>			/* htons() struct sockaddr_in */

#ifndef BIFF_serviceport
#define BIFF_serviceport	COMSATservice
#endif

#ifndef h_0addr_list
#define h_0addr_list	h_addr_list[0]		      /* POSIX struct member */
#endif
