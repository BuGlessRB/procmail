/************************************************************************
 *	multigram - The human mail address reader			*
 *									*
 *	It uses multigrams to intelligently filter out mail addresses	*
 *	from the garbage in the arbitrary mail.				*
 *									*
 *	Seems to be relatively bug free.				*
 *									*
 *	Copyright (c) 1990-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: multigram.c,v 1.7 1992/11/13 12:58:22 berg Exp $";
#endif
static /*const*/char rcsdate[]="$Date: 1992/11/13 12:58:22 $";
#include "includes.h"
#include "sublib.h"
#include "shell.h"
#include "ecommon.h"

#define MIN_addr_len	3		/* shortest meaningful email address */
#define BUFSTEP		16
#define COPYBUF		16384
#define SCALE_WEIGHT	0x7fff

#define DEFmaxgram	4
#define DEFminweight	(SCALE_WEIGHT/4)	      /* sanity cutoff value */
#define DEFbest_matches 2

struct string{char*text,*itext;size_t buflen;};

strnIcmp(a,b,l)const char*a,*b;size_t l;
{ return strncmp(a,b,l);
}

static size_t readstr(file,p)FILE*const file;struct string*p;
{ size_t len=0;int i;
  for(;;)
   { switch(i=getc(file))
      { default:p->text[len]=i;
	   if(++len==p->buflen)
	      p->text=realloc(p->text,p->buflen+=BUFSTEP);
	    continue;
	case ' ':case '\t':case '\n':
	   if(!len)				  /* only skip leading space */
	      continue;
	case EOF:;
      }
     p->text[len]='\0';return len;
   }
}

static char*tstrdup(p)const char*const p;
{ return strcpy(malloc(strlen(p)+1),p);
}

static void lowcase(str)struct string*const str;
{ register char*p;
  for(p=str->itext=tstrdup(str->text);*p;p++)
     if((unsigned)*p-'A'<'Z'-'A')
	*p+='a'-'A';
}

static void elog(a)const char*const a;
{ fputs(a,stderr);
}

void nlog(a)const char*const a;
{ elog("adresses: ");elog(a);
}

static PROGID;

main(minweight,argv)const char*argv[];
{ struct string fuzzstr,hardstr;FILE*hardfile;const char*addit=0;
  struct match{char*fuzz,*hard;int metric;long lentry,offs1,offs2;}
   **best,*curmatch=0;
  unsigned best_matches,maxgram,maxweight,charoffs=0,remov=0,renam=0;
  static const char usage[]=
  "Usage: multigram [-cdr] [-b nnn] [-l nnn] [-w nnn] [-a address] filename\n";
  if(minweight)			      /* sanity check, any arguments at all? */
   { const char*chp;
     minweight=SCALE_WEIGHT;best_matches=maxgram=0;
     while((chp= *++argv)&&*chp=='-')
	for(chp++;;)
	 { int c;
	   switch(c= *chp++)
	    { case 'c':charoffs=1;continue;
	      case 'd':remov=1;continue;
	      case 'r':renam=1;continue;
	      case 'a':
		 if(!*chp&&!(chp= *++argv))
		    goto usg;
		 addit=chp;break;
	      case 'b':case 'l':case 'w':
		{int i;
		 {const char*ochp;
		 if(!*chp&&!(chp= *++argv)||
		  (i=strtol(ochp=chp,(char**)&chp,10),chp==ochp))
		    goto usg;
		 }
		 switch(c)
		  { case 'b':best_matches=i;continue;
		    case 'l':minweight=i;continue;
		    case 'w':maxgram=i;continue;
		  }
		}
	      case HELPOPT1:case HELPOPT2:elog(usage);
		 elog(
 "\t-a address\tadd this address to the list\
\n\t-b nnn\t\tmaximum no. of best matches shown\
\n\t-c\t\tdisplay offsets in characters\
\n\t-d\t\tdelete address from list\
\n\t-l nnn\t\tlower bound metric\
\n\t-r\t\trename address on list\
\n\t-w nnn\t\twindow width used when matching\n");return EX_USAGE;
	      default:goto usg;
	      case '\0':;
	    }
	   break;
	 }
     if(!chp||*++argv||renam+remov+!!addit>1)
	goto usg;
     if(!(hardfile=fopen(chp,remov||renam||addit?"r+":"r")))
      { nlog("Couldn't open \"");elog(chp);elog("\"\n");return EX_IOERR;
      }
   }
  else
usg:
   { elog(usage);return EX_USAGE;
   }
  if(addit)
   { int lnl;long lasttell;
     for(lnl=1,lasttell=0;;)
      { switch(getc(hardfile))
	 { case '\n':
	      if(lnl)
		 break;
	      lasttell=ftell(hardfile);lnl=1;continue;
	   default:lnl=0;continue;
	   case EOF:lasttell=ftell(hardfile);
	 }
	break;
      }
     fseek(hardfile,lasttell,SEEK_SET);fprintf(hardfile,"%s\n",addit);
     printf("Added: %s\n",addit);fclose(hardfile);return EX_OK;
   }
  if(!maxgram)
     maxgram=DEFmaxgram;
  maxgram--;
  if(minweight==SCALE_WEIGHT)
     minweight=DEFminweight;
  if(!best_matches)
     best_matches=DEFbest_matches;
  fuzzstr.text=malloc(fuzzstr.buflen=BUFSTEP);
  hardstr.text=malloc(hardstr.buflen=BUFSTEP);
 {int i;
  best=malloc(best_matches--*sizeof*best);i=best_matches;
  do
   { best[i]=malloc(sizeof**best);best[i]->hard=malloc(1);
     best[i]->fuzz=malloc(1);best[i]->metric= -SCALE_WEIGHT;
   }
  while(i--);
 }
  while(readstr(stdin,&fuzzstr))
   { int meter,maxmetric;size_t fuzzlen;long linentry,offs1,offs2;
    {char*chp,*echp;int parens;
     switch(*(echp=strchr(chp=fuzzstr.text,'\0')-1))
      { case '.':case ',':case ';':case ':':case '?':case '!':*echp--='\0';
      }
     if(!strchr(chp,'@')&&(!strchr(chp,'!')||strchr(chp,'|')||strchr(chp,',')||
      mystrstr(chp,"..",chp+strlen(chp))))
	continue;			  /* apparently not an email address */
     for(parens=0;chp=strchr(chp,'(');chp++,parens++);
     for(chp=fuzzstr.text;chp=strchr(chp,')');chp++,parens--);
     if(*(chp=fuzzstr.text)=='(')
      { if(!parens&&*echp==')')
	 { *echp='\0';goto shftleft;
	 }
	if(parens>0)
shftleft:  tmemmove(chp,chp+1,strlen(chp));
      }
     else if(parens<0&&*echp==')')
	*echp='\0';
     if((fuzzlen=strlen(chp))<MIN_addr_len)
	continue;
     if(!curmatch)
	curmatch=malloc(sizeof*curmatch);
     curmatch->fuzz=tstrdup(chp);curmatch->hard=malloc(1);
     curmatch->metric= -SCALE_WEIGHT;
    }
     lowcase(&fuzzstr);fseek(hardfile,0L,SEEK_SET);
     maxmetric=best[best_matches]->metric;
     for(offs2=linentry=0;offs1=offs2,readstr(hardfile,&hardstr);)
      { size_t minlen;register size_t gramsize;
       {size_t hardlen;
	offs2=ftell(hardfile);linentry++;lowcase(&hardstr);
	if((minlen=hardlen=strlen(hardstr.text))>fuzzlen)
	   minlen=fuzzlen;
	if((gramsize=minlen-1)>maxgram)
	   gramsize=maxgram;
	maxweight=SCALE_WEIGHT/(gramsize+1);
	meter=(int)((unsigned long)SCALE_WEIGHT/2*minlen/
	 (hardlen==minlen?fuzzlen:hardlen))-SCALE_WEIGHT/2;
       }
	do
	 { register lmeter=0;
	  {register const char*fzz,*hrd;
	   fzz=fuzzstr.itext;
	   do
	      for(hrd=hardstr.itext;hrd=strchr(hrd,*fzz);)
		 if(!strncmp(++hrd,fzz+1,gramsize))
		  { lmeter++;break;
		  }
	   while(*(++fzz+gramsize));
	  }
	   if(lmeter)
	    { unsigned weight;
	      meter+=lmeter*(weight=maxweight/(unsigned)(minlen-gramsize));
	      meter-=weight*
	       (unsigned long)((lmeter+=gramsize-minlen)<0?-lmeter:lmeter)/
	       minlen;
	    }
	 }
	while(gramsize--);
	free(hardstr.itext);
	if(meter>maxmetric)
	 { curmatch->metric=maxmetric=meter;curmatch->lentry=linentry;
	   free(curmatch->hard);curmatch->hard=tstrdup(hardstr.text);
	   curmatch->offs1=offs1;curmatch->offs2=offs2;
	 }
      }
     free(fuzzstr.itext);
     if(curmatch->metric>=0)
      { struct match*mp,**mmp;
	free((mp= *(mmp=best+best_matches))->fuzz);free(mp->hard);free(mp);
	while(--mmp>=best&&(mp= *mmp)->metric<curmatch->metric)
	   mmp[1]=mp;
	mmp[1]=curmatch;curmatch=0;
      }
     else
	free(curmatch->fuzz),free(curmatch->hard);
   }
 {int i;struct match*mp;
  for(i=0;i<=best_matches&&(mp=best[i++])->metric>=minweight;)
     printf("%3ld %-34s %5d %-34s\n",charoffs?mp->offs1:mp->lentry,mp->hard,
      mp->metric,mp->fuzz);
  if((mp= *best)->metric>=minweight)
   { struct match*worse;
     if(renam)
      { long line;int i,w1;
	maxweight>>=1;
	for(i=1,line=mp->lentry,w1=mp->metric,worse=0;
	 i<=best_matches&&(mp=best[i++])->metric>=minweight;)
	   if(mp->lentry==line&&mp->metric+maxweight<w1)
	    { goto remv1;
	    }
	for(i=1;i<=best_matches&&(mp=best[i++])->metric>=minweight;)
	   if(mp->metric+maxweight<w1)
remv1:	    { worse=mp;mp= *best;goto remv;
	    }
	nlog("Couldn't find a proper address pair\n");goto norenam;
      }
     if(remov)
remv: { char*buf;long offs1,offs2;size_t readin;
	buf=malloc(COPYBUF);offs1=mp->offs1;offs2=mp->offs2;
	while(fseek(hardfile,offs2,SEEK_SET),
	 readin=fread(buf,1,COPYBUF,hardfile))
	 { offs2=ftell(hardfile);fseek(hardfile,offs1,SEEK_SET);
	   if(buf[readin-1]=='\n')	  /* try and remove some empty lines */
	      while(readin>1&&buf[readin-2]=='\n')	/* at the end, since */
		 readin--;		     /* every time could be the last */
	   fwrite(buf,1,readin,hardfile);offs1=ftell(hardfile);
	 }
	free(buf);fseek(hardfile,offs1,SEEK_SET);
	printf("Removed: %s\n",mp->hard);
	if(renam)
	   fputs(worse->fuzz,hardfile),printf("Added: %s\n",worse->fuzz);
	do putc('\n',hardfile);				   /* erase the tail */
	while(ftell(hardfile)<offs2);
	fclose(hardfile);
      }
     return EX_OK;
   }
 }
  if(remov||renam)
   { nlog("Couldn't even find a single address\n");
   }
norenam:
  return 1;
}
