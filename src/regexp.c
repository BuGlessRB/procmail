/************************************************************************
 *	Custom regular expression library, *fully* egrep compatible	*
 *									*
 *	Seems to be perfect.						*
 *									*
 *	Copyright (c) 1991-1992, S.R. van den Berg, The Netherlands	*
 *	#include "README"						*
 ************************************************************************/
#ifdef RCS
static /*const*/char rcsid[]=
 "$Id: regexp.c,v 1.15 1993/01/13 20:18:03 berg Exp $";
#endif
#include "procmail.h"
#include "robust.h"
#include "shell.h"
#include "regexp.h"

#define R_BEG_GROUP	'('
#define R_OR		'|'
#define R_END_GROUP	')'
#define R_0_OR_MORE	'*'
#define R_0_OR_1	'?'
#define R_1_OR_MORE	'+'
#define R_DOT		'.'
#define R_SOL		'^'
#define R_EOL		'$'
#define R_BEG_CLASS	'['
#define R_NOT_CLASS	'^'
#define R_RANGE		'-'
#define R_END_CLASS	']'
#define R_ESCAPE	'\\'

#define BITS_P_CHAR		8
#define OPB			(1<<BITS_P_CHAR)
#define OPC_EPS			OPB
#define OPC_CLASS		(OPB+1)
#define OPC_DOT			(OPB+2)
#define OPC_BOTEXT		(OPB+3)
#define OPC_FIN			(OPB+4)

#define bit_type		unsigned
#define bit_bits		(sizeof(bit_type)*8)
#define bit_index(which)	((unsigned)(which)/bit_bits)
#define bit_mask(which)		((unsigned)1<<(unsigned)(which)%bit_bits)
#define bit_toggle(name,which)	(name[bit_index(which)]^=bit_mask(which))
#define bit_test(name,which)	(!!(name[bit_index(which)]&bit_mask(which)))
#define bit_set(name,which,value)	\
 (value?(name[bit_index(which)]|=bit_mask(which)):\
 (name[bit_index(which)]&=~bit_mask(which)))
#define bit_field(name,size)	bit_type name[((size)+bit_bits-1)/bit_bits]

#define SZ(x)		(sizeof(struct x))
#define Ceps		(struct eps*)
#define epso(to,add)	(Ceps((char*)(to)+(add)))
#define ii		(aleps.topc)
#define jjp		(aleps.tnext)

/* the spawn and stack members are reused in the normal opcodes as pc fields */
static struct eps*r;
static struct{unsigned topc;struct eps*tnext;}aleps;
static uchar*p,*cachea,*cachep;
static size_t cacher;
static ignore_case;

struct chclass {unsigned opc_;struct eps*stack_,*spawn_,*next_;
 bit_field(c,OPB);};

static void puteps(spot,to,aswell)struct eps*const spot;  /* epsilon transit */
 const struct eps*const to,*const aswell;
{ spot->opc=OPC_EPS;spot->next=to!=spot?Ceps to:Ceps aswell;
  spot->spawn=aswell!=spot?Ceps aswell:Ceps to;spot->stack=0;
}

static void putneps(spot,to)struct eps*const spot;const struct eps*const to;
{ puteps(spot,to,spot+1);
}

#define rAc	(((struct chclass*)r)->c)

static void bseti(i,j)unsigned i;const int j;
{ bit_set(rAc,i,j);			   /* mark 'i' as being in the class */
  if(ignore_case)				  /* mark the other case too */
   { if(i-'A'<'Z'-'A')						/* uppercase */
	i+='a'-'A';
     else if(i-'a'<'z'-'a')					/* lowercase */
	i-='a'-'A';
     else return;						  /* no case */
     bit_set(rAc,i,j);
   }
}

static void por P((const struct eps*const e));

static void psimp(e)const struct eps*const e;
{ switch(*p)
   { case R_BEG_GROUP:p++;por(e);return;	  /* not so simple after all */
     case R_BEG_CLASS:					   /* a simple class */
      { unsigned i,j=R_NOT_CLASS==*++p;
	if(e)
	 { r->opc=OPC_CLASS;r->next=Ceps e;r->spawn=r->stack=0;
	   i=maxindex(rAc);
	   do rAc[i]=j?~0:0;			     /* preset the bit field */
	   while(i--);
	 }
	if(j)					  /* skip the 'not' modifier */
	 { p++;
	   if(e)
	      bit_toggle(rAc,'\n');
	 }
	if(*p==R_END_CLASS)	  /* right at the start, cannot mean the end */
	 { p++;
	   if(e)
	      i=R_END_CLASS,bit_toggle(rAc,R_END_CLASS);
	 }
	else if(*p==R_RANGE)				/* take it literally */
	 { p++;
	   if(e)
	      i=R_RANGE,bit_toggle(rAc,R_RANGE);
	 }
	for(;;p++)
	 { switch(*p)
	    { case R_END_CLASS:p++;
	      case '\0':r=epso(r,SZ(chclass));return;
	      case R_RANGE:
		 switch(*++p)
		  { default:
		       if(e)
			  while(++i<*p)		    /* mark all in the range */
			     bseti(i,!j);
		       break;
		    case '\0':case R_END_CLASS:p--;		/* literally */
		  }
	    }
	   if(e)
	      bseti(i= *p,!j);		      /* a normal character, mark it */
	 }
      }
     case '\0':return;
     case R_DOT:			 /* matches everything but a newline */
	if(e)
	 { r->opc=OPC_DOT;goto fine;
	 }
	goto fine2;
     case R_EOL:case R_SOL:		      /* match a newline (in effect) */
	if(p[1]==R_SOL)
	 { p++;
	   if(e)
	    {  r->opc=OPC_BOTEXT;goto fine;
	    }
	 }
	else if(e)
	 { r->opc='\n';goto fine;
	 }
	goto fine2;
     case R_ESCAPE:					  /* quote something */
	if(!*++p)					 /* nothing to quote */
	   p--;
   }
  if(e)						      /* a regular character */
   { r->opc=ignore_case&&(unsigned)*p-'A'<'Z'-'A'?*p+'a'-'A':*p;
fine:
     r->next=Ceps e;r->spawn=r->stack=0;
   }
fine2:
  p++;r++;
}

#define EOS(x)	(jjp?jjp:(x))

static void pnorm(e)const struct eps*const e;
{ void*pold;struct eps*rold;
  for(;;)
   { pold=p;rold=r;psimp(Ceps 0);ii= *p;		    /* skip it first */
     jjp=p[1]==R_OR||p[1]==R_END_GROUP||!p[1]?Ceps e:Ceps 0;
     if(e)
	p=pold,pold=r;
     switch(ii)			   /* check for any of the postfix operators */
      { case R_0_OR_MORE:r++;
	   if(e)			  /* first an epsilon, then the rest */
	      putneps(rold,EOS(r)),r=rold+1,psimp(rold);
	   goto incagoon;
	case R_1_OR_MORE:				   /* first the rest */
	   if(e)				      /* and then an epsilon */
	      puteps(r,rold,EOS(r+1)),r=rold,psimp(Ceps pold);
	   r++;goto incagoon;
	case R_0_OR_1:r++;
	   if(e)			  /* first an epsilon, then the rest */
	      putneps(rold,r=EOS(r)),pold=r,r=rold+1,psimp(Ceps pold);
incagoon:  switch(*++p)			/* at the end of this group already? */
	    { case R_OR:case R_END_GROUP:case '\0':return;
	    }
	   continue;				 /* regular end of the group */
	case R_OR:case R_END_GROUP:case '\0':
	   if(e)
	      r=rold,psimp(e);
	   return;
      }
     if(e)			/* no fancy postfix operators, plain vanilla */
	r=rold,psimp(Ceps pold);
   }
}

static void por(e)const struct eps*const e;
{ uchar*pvold;struct eps*rvold;
  if(!e)
   { rvold=r;
     if(cachea==(pvold=p))
      { p=cachep;r=epso(rvold,cacher);return;
      }
   }
  for(;;)
   { uchar*pold;struct eps*rold;
     for(pold=p,rold=r;;)
      { switch(*p)
	 { default:pnorm(Ceps 0);r=rold;continue;     /* still in this group */
	   case '\0':case R_END_GROUP:	       /* found the end of the group */
	      if(p==pold)				 /* empty 'or' group */
	       { if(e)
		    puteps(r,e,e);	       /* misused epsilon as branch, */
		 r++;		/* let the optimiser (fillout()) take it out */
	       }
	      else
		 p=pold,pnorm(e);			/* normal last group */
	      if(*p)
		 p++;
	      if(!e)
		 cachea=pvold,cachep=p,cacher=(char*)r-(char*)rvold;
	      return;
	   case R_OR:r++;
	      if(p==pold)				 /* empty 'or' group */
	       { if(e)
		    putneps(rold,e);			  /* special epsilon */
	       }
	      else
	       { p=pold;pnorm(e);	      /* normal 'or' group, first an */
		 if(e)				   /* epsilon, then the rest */
		    putneps(rold,r);
	       }
	      p++;
	 }
	break;
      }
   }
}

static void findandrep(old,newv)register struct eps**const old;
 struct eps*const newv;
{ register struct eps*i,*t= *old;			   /* save the value */
  for(i=r;i->opc!=OPC_FIN;)	     /* change all pointers from *old to new */
   { if(i->next==t)
	i->next=newv;
     if(i->spawn==t)
	i->spawn=newv;
     switch(i->opc)
      { case OPC_CLASS:i=epso(i,SZ(chclass));break;
	default:i++;
      }
   }
  *old=t;
}

#define drs(m)	(*(struct eps**)((char*)*stack+(ioffsetof(struct eps,m)^ofs)))

static cstack(stack,ofs)struct eps**const stack;
{ if(drs(next)->stack==Ceps p)
   { findandrep(*stack,drs(next));*stack=drs(spawn);return 1;
   }
  return 0;
}
    /* break up any closed epsilon circles, otherwise they can't be executed */
static fillout(stack)struct eps**const stack;
{ if((*stack)->opc!=OPC_EPS||(*stack)->stack)
     return 0;
  (*stack)->stack=Ceps p;			    /* mark this one as used */
#define RECURS(nxt)	\
  do\
     if(cstack(stack,ioffsetof(struct eps,nxt)^ioffsetof(struct eps,next)))\
	return 1;\
  while(fillout(&(*stack)->nxt))
  RECURS(next);RECURS(spawn);return 0;				  /* recurse */
}

struct eps*bregcomp(a,ign_case)const char*a;
{ struct eps*st;size_t i;      /* first a trial run, determine memory needed */
  p=(uchar*)a;ignore_case=ign_case;r=Ceps&aleps;cachea=0;por(Ceps 0);
  st=r=
   malloc((i=(char*)r-(char*)&aleps)+ioffsetof(struct eps,stack)+sizeof r);
  p=(uchar*)a;por(epso(st,i));r->opc=OPC_FIN;r->stack=0;	  /* compile */
  for(r=st;;)				 /* simplify the compiled code (i.e. */
     switch(st->opc)		      /* take out cyclic epsilon references) */
      { case OPC_FIN:return r;					 /* finished */
	case OPC_CLASS:st=epso(st,SZ(chclass));break;		     /* skip */
	case OPC_EPS:p=(uchar*)st;fillout(&st);		       /* check tree */
	default:st++;						 /* skip too */
      }
}

#define XOR1		\
 (ioffsetof(struct eps,spawn)^ioffsetof(struct eps,stack))
#define PC(this,t)	(*(struct eps**)((char*)(this)+(t)))

char*bregexec(code,text,len,ign_case)struct eps*code;const uchar*const text;
 size_t len;
{ register struct eps*reg,*stack,*other,*thiss;unsigned i,th1,ot1;
  const uchar*str;struct eps*initstack,*initcode;
  initstack=0;
  if((initcode=code)->opc==OPC_EPS)
     initcode=(initstack=code)->next,code->stack=0;
  thiss= --code;th1=ioffsetof(struct eps,spawn);
  ot1=ioffsetof(struct eps,stack);str=text-1;len++;i='\n';goto setups;
  do			      /* make sure any beginning-of-line-hooks catch */
   { i= *++str;				 /* get the next real-text character */
     if(ign_case&&i-'A'<'Z'-'A')
	i+='a'-'A';			     /* transmogrify it to lowercase */
lastrun:				     /* switch this & other pc-stack */
     th1^=XOR1;ot1^=XOR1;thiss=other;
setups:
     other=code;stack=initstack;reg=initcode;goto nostack;
     do					 /* pop next entry off this pc-stack */
      { thiss=PC(reg=thiss,th1);PC(reg,th1)=0;reg=reg->next;goto nostack;
	do				/* pop next entry off the work-stack */
	 { for(reg=stack->spawn,stack=stack->stack;;)
nostack:    { switch(reg->opc-OPB)  /* push spawned branch on the work-stack */
	       { default:
		    if(i==reg->opc)		  /* regular character match */
		       goto yep;
		    break;
		 case OPC_EPS-OPB:reg->stack=stack;reg=(stack=reg)->next;
		    continue;
		 case OPC_FIN-OPB:	   /* hurray!  complete regexp match */
		    return(char*)str;		/* return one past the match */
		 case OPC_BOTEXT-OPB:
		    if(str<text)	       /* only at the very beginning */
		       goto yep;
		    break;
		 case OPC_CLASS-OPB:
		    if(bit_test(((struct chclass*)reg)->c,i))
		       goto yep;		       /* character in class */
		    break;
		 case OPC_DOT-OPB:			     /* dot-wildcard */
		    if(i!='\n')
yep:		       if(!PC(reg,ot1))		     /* state not yet pushed */
			  PC(reg,ot1)=other,other=reg; /* push location onto */
	       }					   /* other pc-stack */
	      break;
	    }
	 }
	while(stack);			      /* the work-stack is not empty */
      }
     while(thiss!=code);		       /* this pc-stack is not empty */
   }
  while(--len);					     /* still text to search */
  if(ign_case!=2)					      /* out of text */
   { ign_case=2;len=1;str++;goto lastrun;	 /* check if we just matched */
   }
  return 0;							 /* no match */
}
