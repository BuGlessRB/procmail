/* $Id$ */

const char
 *sputenv P((const char*const a)),
 *eputenv P((const char*const src,char*const dst)),
 *tgetenv P((const char*const a));
void
 primeStdout P((const char*const varname)),
 retStdout P((char*const newmyenv,const int fail,const int unset)),
 retbStdout P((char*const newmyenv)),
 appendlastvar P((const char*const value)),
 cleanupenv P((int preserve)),
 initdefenv Q2((auth_identity*pass,const char*fallback,int do_presets,
  const char*mode)),
 asenv P((const char*const chp)),
 setval P((const char*const name,const char*const value)),
 setdef P((const char*const name,const char*const value)),
 setlastfolder P((const char*const folder)),
 allocbuffers Q((size_t lineb,int setenv)),
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
extern char*Stdout;
extern const char lastfolder[],maildir[],scomsat[],offvalue[];
extern int didchd;
