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
/*$Id: header.h,v 1.22 1993/06/28 16:23:21 berg Exp $*/

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
 x400received[]=	"X400-Received:",
 priority[]=		"Priority:",			    /* ELM extension */
 fcc[]=			"Fcc:",				   /* Mush extension */
 resent[]=		"Resent:",			     /* MH extension */
 forwarded[]=		"Forwarded:",				/* ditto ... */
 replied[]=		"Replied:",
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
 nntppostinghost[]=	"NNTP-Posting-Host:",
 submittedby[]=		"Submitted-by:",
 title[]=		"Title:",	      /* antiquated USENET extension */
 aRticleid[]=		"Article-I.D.:",			/* ditto ... */
 posted[]=		"Posted:",
 relayversion[]=	"Relay-Version:",
 cnttype[]=		"Content-Type:",	       /* Internet extension */
 encoding[]=		"Encoding:",				/* ditto ... */
 cntmd5[]=		"Content-MD5:",
 mimeversion[]=		"MIME-Version:",		   /* MIME extension */
 cnttransferenc[]=	"Content-Transfer-Encoding:",		/* ditto ... */
 cntid[]=		"Content-ID:",
 cntdescription[]=	"Content-Description:",
 cntdisposition[]=	"Content-Disposition:",
 transportoptions[]=	"Transport-Options:",	    /* SysV mailer extension */
 defltoptions[]=	"Default-Options:",
 cntlength[]=		"Content-Length:",
 rference[]=		"Reference:",
 msgtype[]=		"Message-Type:",
 autoforwardedfrom[]=	"Auto-Forwarded-From:",
 autofcount[]=		"Auto-Forward-Count:",
 endofheader[]=		"End-of-Header:",
 orgaforwfrom[]=	"Original-Auto-Forwarded-From:",
 orgdate[]=		"Original-Date:",
 notdeliveredto[]=	"Not-Delivered-To:",
 reportversion[]=	"Report-Version:",
 status[]=		"Status:",			 /* mailer extension */
 readreceiptto[]=	"Read-Receipt-To:";	  /* miscellaneous extension */

static const struct {const char*hedr;int lnr;}cdigest[]=
{ bsl(returnpath),bsl(received),bsl(replyto),bsl(Fromm),bsl(sender),
  bsl(res_replyto),bsl(res_from),bsl(res_sender),bsl(date),bsl(res_date),
  bsl(to),bsl(res_to),bsl(cc),bsl(res_cc),bsl(bcc),bsl(res_bcc),bsl(messageid),
  bsl(res_messageid),bsl(inreplyto),bsl(references),bsl(keywords),bsl(subject),
  bsl(scomments),bsl(ncrypted),bsl(errorsto),bsl(retreceiptto),
  bsl(precedence),bsl(fullname),bsl(postddate),bsl(recvddate),bsl(mssage),
  bsl(text),bsl(via),bsl(x400received),bsl(priority),bsl(fcc),bsl(resent),
  bsl(forwarded),bsl(replied),bsl(article),bsl(path),bsl(summary),
  bsl(organisation),bsl(aorganization),bsl(newsgroups),bsl(followupto),
  bsl(approved),bsl(lines),bsl(expires),bsl(control),bsl(distribution),
  bsl(xref),bsl(originator),bsl(nntppostinghost),bsl(submittedby),bsl(title),
  bsl(aRticleid),bsl(posted),bsl(relayversion),bsl(cnttype),bsl(encoding),
  bsl(cntmd5),bsl(mimeversion),bsl(cnttransferenc),bsl(cntid),
  bsl(cntdescription),bsl(cntdisposition),bsl(transportoptions),
  bsl(defltoptions),bsl(cntlength),bsl(rference),bsl(msgtype),
  bsl(autoforwardedfrom),bsl(autofcount),bsl(endofheader),bsl(orgaforwfrom),
  bsl(orgdate),bsl(notdeliveredto),bsl(reportversion),bsl(status),
  bsl(readreceiptto)
};
