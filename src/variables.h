/* $Id: variables.h,v 1.4 2000/11/18 06:49:07 guenther Exp $ */

const char
 *sputenv P((const char*const a)),
 *eputenv P((const char*const src,char*const dst)),
 *tgetenv P((const char*const a));
void
 primeStdout P((const char*const varname)),
 retStdout P((char*const newmyenv,int unset)),
 retbStdout P((char*const newmyenv)),
 postStdout P((void)),
 cleanupenv P((int preserve)),
 initdefenv Q((auth_identity*pass,const char*fallback,int do_presets)),
 asenv P((const char*const chp)),
 setdef P((const char*const name,const char*const value)),
 setlastfolder P((const char*const folder)),
 mallocbuffers Q((size_t lineb,int setenv)),
 setmaildir P((const char*const newdir)),
 setoverflow P((void));
int
 asenvcpy P((char*src)),
 setexitcode P((int trapisset)),
 alphanum P((const unsigned c));
char
 *gobenv P((char*chp,char*end));
long
 renvint P((const long i,const char*const env));

extern long Stdfilled;
extern const char lastfolder[],maildir[];
extern int didchd;
