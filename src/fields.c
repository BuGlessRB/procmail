/************************************************************************
 *	Routines to deal with the header-field objects in formail	*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: fields.c,v 1.10 1993/04/27 17:33:55 berg Exp $";
#endif
#include "includes.h"
#include "formail.h"
#include "sublib.h"
#include "shell.h"
#include "common.h"
#include "fields.h"
#include "ecommon.h"
#include "formisc.h"

struct field*findf(p,hdr)const struct field*const p,*hdr;
{ size_t i;char*chp;		/* find a field in the linked list of fields */
  for(i=p->id_len,chp=(char*)p->fld_text;hdr;hdr=hdr->fld_next)
     if(i==hdr->id_len&&!strnIcmp(chp,hdr->fld_text,i))	 /* case insensitive */
	return(struct field*)hdr;
  return(struct field*)0;
}

struct field**addfield(pointer,text,totlen)register struct field**pointer;
 const char*const text;const size_t totlen;    /* add field to a linked list */
{ register struct field*p;
  while(*pointer)			      /* skip to the end of the list */
     pointer= &(*pointer)->fld_next;
  (*pointer=p=malloc(FLD_HEADSIZ+totlen))->fld_next=0;	 /* create the field */
  p->id_len=breakfield(text,totlen);		  /* and copy field contents */
  tmemmove(p->fld_text,text,p->tot_len=totlen);return pointer;
}

void concatenate(fldp)struct field*const fldp;
{ register char*p;register size_t l;	    /* concatenate a continued field */
  l=fldp->tot_len;p=fldp->fld_text;
  while(l--)
     if(*p++=='\n'&&l)	     /* by substituting all newlines except the last */
	p[-1]=' ';
}

void renfield(pointer,oldl,newname,newl)struct field**const pointer;
 const size_t oldl,newl;const char*const newname;	    /* rename fields */
{ struct field*p;size_t i;char*chp;
  i=(p= *pointer)->tot_len-oldl;	      /* length of what we will keep */
  *pointer=p=realloc(p,FLD_HEADSIZ+(p->tot_len=i+newl));chp=p->fld_text;
  tmemmove(chp+newl,chp+oldl,i);tmemmove(chp,newname,newl);   /* shove, copy */
}

static void extractfield(p)register struct field*p;
{ if(xheader||Xheader)					 /* extracting only? */
   { if(findf(p,xheader))			   /* extract field contents */
      { putssn(p->fld_text+p->id_len,p->tot_len-p->id_len);return;
      }
     if(!findf(p,Xheader))				   /* extract fields */
	return;
   }
  lputssn(p->fld_text,p->tot_len);		      /* display it entirely */
}

void flushfield(pointer)register struct field**pointer;	 /* delete and print */
{ register struct field*p,*q;				   /* them as you go */
  for(p= *pointer,*pointer=0;p;p=q)
     q=p->fld_next,extractfield(p),free(p);
}

void dispfield(p)register const struct field*p;
{ for(;p;p=p->fld_next)			     /* print list non-destructively */
     if(p->id_len<p->tot_len-1)			 /* any contents to display? */
	extractfield(p);
}

readhead P((void))  /* try and append one valid field to rdheader from stdin */
{ getline();
  if(!eqFrom_(buf))				    /* it's not a From_ line */
   { if(!breakfield(buf,buffilled))	   /* not the start of a valid field */
	return 0;
     for(;;getline())		      /* get the rest of the continued field */
      { switch(buflast)			     /* will this line be continued? */
	 { case ' ':case '\t':continue;			  /* yep, it sure is */
	 }
	break;
      }
   }
  else if(rdheader)
     return 0;				       /* the From_ line was a fake! */
  addbuf();return 1;		  /* phew, got the field, add it to rdheader */
}

void addbuf P((void))
{ addfield(&rdheader,buf,buffilled);buffilled=0;
}
