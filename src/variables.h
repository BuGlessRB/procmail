/* $Id: variables.h,v 1.1 2000/10/23 09:04:26 guenther Exp $ */

extern long Stdfilled;

const char
 *sputenv P((const char*const a)),
 *eputenv P((const char*const src,char*const dst));
void
 primeStdout P((const char*const varname)),
 retStdout P((char*const newmyenv,int unset)),
 retbStdout P((char*const newmyenv)),
 postStdout P((void)),
 cleanupenv P((int preserve)),
 setupenv Q((auth_identity*pass,const char*fallback,int do_presets));
