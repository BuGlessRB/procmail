/************************************************************************
 *	A heap sort implementation					*
 *									*
 *	Copyright (c) 1994-1997, S.R. van den Berg, The Netherlands	*
 *	#include "../README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: hsort.c,v 1.7 1997/04/03 01:58:43 srb Exp $";
#endif
#include "includes.h"
#include "hsort.h"

typedef uchar*bytep;
#define fpcmp(p1,p2)	((*fcmp)(p1,p2))

static void swap(p1,p2,len)bytep p1,p2;size_t len;
{ register bytep q1=p1,q2=p2;register size_t l=len;
  do
   { register unsigned i;
     i= *q1;*q1= *q2;*q2=i;		/* swap the memory regions, bytewise */
   }
  while(q1++,q2++,--l);
}
				/* heapsort, drop-in replacement for qsort() */
void hsort(base,nelem,width,fcmp)void*base;size_t nelem,width;
 register int(*fcmp)P((const void*,const void*));
{ register bytep rroot,root;register size_t leafoffset;
  if((leafoffset=nelem)<2)		      /* we won't do it for less :-) */
     return;		       /* actually, we only *need* to check for zero */
  for(root=(rroot=base)+width*(leafoffset>>1),
   leafoffset=width*(leafoffset-1);;)
   { if(root>rroot)				      /* tree still growing? */
	root-=width;			      /* gradually build up the tree */
     else
      { swap(root,root+leafoffset,width);     /* move the highest into place */
	if(!(leafoffset-=width))			  /* shrink the tree */
	   return;						   /* ready! */
      }
     ;{ register bytep parent;
	for(parent=root;;)		/* sift the root element to its spot */
	 { register bytep child;size_t childoffset;
	   child=rroot+(childoffset=(parent-rroot<<1)+width);  /* find child */
	   if(childoffset<leafoffset)		     /* child within bounds? */
	    { if(fpcmp(child,child+width)<0)	     /* pick highest sibling */
		 child+=width;
docmp:	      if(fpcmp(parent,child)<0)		 /* parent lower than child? */
	       { swap(parent,child,width);parent=child;
		 continue;
	       }					    /* delve deeper! */
	    }
	   else if(childoffset==leafoffset)		      /* no sibling? */
	      goto docmp;		    /* then compare it with this one */
	   break;					  /* it settled down */
	 }
      }
   }
}
