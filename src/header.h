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
/*$Id: header.h,v 1.27 1994/02/22 17:25:05 berg Exp $*/

X(returnpath,		"Return-Path:")				  /* RFC 822 */
X(received,		"Received:")				/* ditto ... */
X(replyto,		"Reply-To:")
X(Fromm,		"From:")
X(sender,		"Sender:")
X(res_replyto,		"Resent-Reply-To:")
X(res_from,		"Resent-From:")
X(res_sender,		"Resent-Sender:")
X(date,			"Date:")
X(res_date,		"Resent-Date:")
X(to,			"To:")
X(res_to,		"Resent-To:")
X(cc,			"Cc:")
X(res_cc,		"Resent-Cc:")
X(bcc,			"Bcc:")
X(res_bcc,		"Resent-Bcc:")
X(messageid,		"Message-ID:")
X(res_messageid,	"Resent-Message-ID:")
X(inreplyto,		"In-Reply-To:")
X(references,		"References:")
X(keywords,		"Keywords:")
X(subject,		"Subject:")
X(scomments,		"Comments:")
X(ncrypted,		"Encrypted:")
X(errorsto,		"Errors-To:")		       /* sendmail extension */
X(retreceiptto,		"Return-Receipt-To:")			/* ditto ... */
X(precedence,		"Precedence:")
X(fullname,		"Full-Name:")
X(postddate,		"Posted-Date:")
X(recvddate,		"Received-Date:")
X(mssage,		"Message:")
X(text,			"Text:")
X(via,			"Via:")
X(x400received,		"X400-Received:")
X(x400originator,	"X400-Originator:")
X(x400rcipients,	"X400-Recipients:")
X(x400mtsidentifier,	"X400-Mts-Identifier:")
X(x400contenttype,	"X400-Content-Type:")
X(priority,		"Priority:")			    /* ELM extension */
X(fcc,			"Fcc:")				   /* Mush extension */
X(resent,		"Resent:")			     /* MH extension */
X(forwarded,		"Forwarded:")				/* ditto ... */
X(replied,		"Replied:")
X(article,		"Article:")			 /* USENET extension */
X(path,			"Path:")				/* ditto ... */
X(summary,		"Summary:")
X(organisation,		"Organisation:")
X(aorganization,	"Organization:")
X(newsgroups,		"Newsgroups:")
X(followupto,		"Followup-To:")
X(approved,		"Approved:")
X(lines,		"Lines:")
X(expires,		"Expires:")
X(control,		"Control:")
X(distribution,		"Distribution:")
X(xref,			"Xref:")
X(originator,		"Originator:")
X(nntppostinghost,	"NNTP-Posting-Host:")
X(submittedby,		"Submitted-by:")
X(title,		"Title:")	      /* antiquated USENET extension */
X(aRticleid,		"Article-I.D.:")			/* ditto ... */
X(posted,		"Posted:")
X(relayversion,		"Relay-Version:")
X(cnttype,		"Content-Type:")	       /* Internet extension */
X(encoding,		"Encoding:")				/* ditto ... */
X(cntmd5,		"Content-MD5:")
X(mimeversion,		"MIME-Version:")		   /* MIME extension */
X(cnttransferenc,	"Content-Transfer-Encoding:")		/* ditto ... */
X(cntid,		"Content-ID:")
X(cntdescription,	"Content-Description:")
X(cntdisposition,	"Content-Disposition:")
X(transportoptions,	"Transport-Options:")	    /* SysV mailer extension */
X(defltoptions,		"Default-Options:")
X(cntlength,		"Content-Length:")
X(rference,		"Reference:")
X(msgtype,		"Message-Type:")
X(autoforwardedfrom,	"Auto-Forwarded-From:")
X(autofcount,		"Auto-Forward-Count:")
X(endofheader,		"End-of-Header:")
X(orgafrom,		"Original-From:")
X(orgaforwfrom,		"Original-Auto-Forwarded-From:")
X(orgdate,		"Original-Date:")
X(notdeliveredto,	"Not-Delivered-To:")
X(reportversion,	"Report-Version:")
X(status,		"Status:")			 /* mailer extension */
X(mailfrom,		"Mail-from:")		    /* emacs BABYL extension */
X(readreceiptto,	"Read-Receipt-To:")	  /* miscellaneous extension */
