/************************************************************************
 *									*
 *	Known fields when formail is splitting messages (the first	*
 *	"-m nnn" fields encountered should be among them or one of	*
 *	the special From_, Article_ or X- fields).			*
 *									*
 *	If you need to add one (be sure to update "cdigest" below as	*
 *	well!), drop me a mail, I might be interested in including	*
 *	it in the next release.						*
 *									*
 ************************************************************************/
/*$Id: header.h,v 1.12 1993/01/26 14:49:52 berg Exp $*/

static const char
 returnpath[]=		"Return-Path:",				  /* RFC 822 */
 received[]=		"Received:",				/* ditto ... */
 replyto[]=		"Reply-To:",
 Fromm[]=		"From:",
 sender[]=		"Sender:",
 res_replyto[]=		"Resent-Reply-To:",
 res_from[]=		"Resent-From:",
 res_sender[]=		"Resent-Sender:",
 date[]=		"Date:",
 res_date[]=		"Resent-Date:",
 to[]=			"To:",
 res_to[]=		"Resent-To:",
 cc[]=			"Cc:",
 res_cc[]=		"Resent-Cc:",
 bcc[]=			"Bcc:",
 res_bcc[]=		"Resent-Bcc:",
 messageid[]=		"Message-ID:",
 res_messageid[]=	"Resent-Message-ID:",
 inreplyto[]=		"In-Reply-To:",
 references[]=		"References:",
 keywords[]=		"Keywords:",
 subject[]=		"Subject:",
 scomments[]=		"Comments:",
 ncrypted[]=		"Encrypted:",
 errorsto[]=		"Errors-To:",		       /* sendmail extension */
 retreceiptto[]=	"Return-Receipt-To:",			/* ditto ... */
 precedence[]=		"Precedence:",
 fullname[]=		"Full-Name:",
 postddate[]=		"Posted-Date:",
 recvddate[]=		"Received-Date:",
 mssage[]=		"Message:",
 text[]=		"Text:",
 via[]=			"Via:",
 priority[]=		"Priority:",			    /* ELM extension */
 fcc[]=			"Fcc:",				   /* Mush extension */
 article[]=		"Article:",			 /* USENET extension */
 path[]=		"Path:",				/* ditto ... */
 summary[]=		"Summary:",
 organisation[]=	"Organisation:",
 aorganization[]=	"Organization:",
 newsgroups[]=		"Newsgroups:",
 followupto[]=		"Followup-To:",
 approved[]=		"Approved:",
 lines[]=		"Lines:",
 expires[]=		"Expires:",
 control[]=		"Control:",
 distribution[]=	"Distribution:",
 xref[]=		"Xref:",
 originator[]=		"Originator:",
 nntppostinghost[]=	"Nntp-Posting-Host:",
 title[]=		"Title:",	      /* antiquated USENET extension */
 aRticleid[]=		"Article-I.D.:",			/* ditto ... */
 posted[]=		"Posted:",
 cnttype[]=		"Content-Type:",	       /* Internet extension */
 encoding[]=		"Encoding:",				/* ditto ... */
 mimeversion[]=		"MIME-Version:",		   /* MIME extension */
 cnttransferenc[]=	"Content-Transfer-Encoding:",		/* ditto ... */
 cntid[]=		"Content-ID:",
 cntdescription[]=	"Content-Description:",
 transportoptions[]=	"Transport-Options:",	    /* SysV mailer extension */
 defltoptions[]=	"Default-Options:",
 cntlength[]=		"Content-Length:",
 status[]=		"Status:";			 /* mailer extension */

static const struct {const char*hedr;int lnr;}cdigest[]=
{ bsl(returnpath),bsl(received),bsl(replyto),bsl(Fromm),bsl(sender),
  bsl(res_replyto),bsl(res_from),bsl(res_sender),bsl(date),bsl(res_date),
  bsl(to),bsl(res_to),bsl(cc),bsl(res_cc),bsl(bcc),bsl(res_bcc),bsl(messageid),
  bsl(res_messageid),bsl(inreplyto),bsl(references),bsl(keywords),bsl(subject),
  bsl(scomments),bsl(ncrypted),bsl(errorsto),bsl(retreceiptto),
  bsl(precedence),bsl(fullname),bsl(postddate),bsl(recvddate),bsl(mssage),
  bsl(text),bsl(via),bsl(priority),bsl(fcc),bsl(article),bsl(path),
  bsl(summary),bsl(organisation),bsl(aorganization),bsl(newsgroups),
  bsl(followupto),bsl(approved),bsl(lines),bsl(expires),bsl(control),
  bsl(distribution),bsl(xref),bsl(originator),bsl(nntppostinghost),bsl(title),
  bsl(aRticleid),bsl(posted),bsl(cnttype),bsl(encoding),bsl(mimeversion),
  bsl(cnttransferenc),bsl(cntid),bsl(cntdescription),bsl(transportoptions),
  bsl(defltoptions),bsl(cntlength),bsl(status)
};
