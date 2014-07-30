/****************************************************************************/
/*  Proto.h                                                                 */
/****************************************************************************/

#ifdef NEWMSGTAB

#define chkptr(foo)\
        if ((foo) != NULL) \
            cPrintf("\a\nDangling " #foo " = %Fp (%s:%u)\n",\
                    (foo), __FILE__,__LINE__)



/***** auxmem.asm *****/
int addToEMS(void);
int addToVirt(void);
int addToXMS(void);
void disposeAuxMem(char *vmfile);
void setUpAuxMem(char *vmfile);
int EMStoHeap(void *whereto, int whatblock);
int VirttoHeap(void *whereto, int whatblock);
int XMStoHeap(void *whereto, int whatblock);
int HeapToXMS(int whatblock, void *whereto);
int HeapToEMS(int whatblock, void *whereto);
int HeapToVirt(int whatblock, void *whereto);

/***** msgtab.c *****/
void createMsgTab(void);
struct messagetable *getMsgTab(ulong slot);
struct msgflags *getFlags(ulong slot);
void destroyMsgTab(void);
struct messagetable *getMsgTab(ulong slot);
ulong getLocation(ulong slot);
int getRoomNum(ulong slot);
int getToHash(ulong slot);
int getAuthHash(ulong slot);
uint getOriginID(ulong slot);

#endif


/* SER.ASM */
extern void  INITRS(int Port, int Baud, int Stops, int Parity, int Length, int cCts);
extern void  DEINITRS(void);
extern void  FLUSHRS(void);
extern int   STATRS(void);
extern int   GETRS(void);
extern void  PUTRS(char ch);
extern void  DTRRS(int dtr);
extern int   CARRSTATRS(void);
extern int   RINGSTATRS(void);
extern int   STATCON(void);
extern int   GETCON(void);
extern void  PUTCON(char ch);

/* ACCOUNT.C */
extern void clearthisAccount(void);
extern void logincrement(void);
extern int logincheck(void);
extern void negotiate(void);
extern void newaccount(void);
extern void unlogthisAccount(void);
extern void updatebalance(void);
extern void calc_trans(long time1, long time2, char trans);

/* APLIC.C */
extern void ExeAplic(void);
extern void shellescape(int super);
extern void writeAplFile(void);
extern void readAplFile(void);
extern void wxsnd(char *path, char *file, char trans);
extern void wxrcv(char *path, char *file, char trans);
extern BOOL execDoor(char c);
extern void apsystem(char *stuff);

/* CARRIER.C */
extern BOOL haveCarrier(void);
extern void carrdetect(void);
extern void carrloss(void);
extern BOOL checkCR(void);
extern int findbaud(void);
extern void ringdetectbaud(void);
extern int smartbaud(void);
extern int getModStr(char *str);
extern int carrier(void);
extern void checkring(void);

/* CHAT.C */
extern void chat(void);
extern void ringSysop(void);

/* CONFIG.C */
extern void buildhalls(void);
extern void buildroom(void);
extern void clearaccount(void);
extern void configcit(void);
extern void illegal(const char *fmt, ...);
extern void initfiles(void);
extern void logInit(void);
extern int logSort(struct lTable *s1, struct lTable *s2);
extern void readaccount(void);
extern void readprotocols(void);
extern void readconfig(char ignore);
extern void RoomTabBld(void);
extern void zapGrpFile(void);
extern void zapHallFile(void);
extern int zapLogFile(void);
extern int zapRoomFile(void);

/* CONSOLE.C */
extern void doccr(void);
extern void fkey(void);
extern BOOL KBReady(void);
extern int ciChar(void);
extern void outCon(char c);
extern void cPrintf(char *fmt, ...);
extern void cCPrintf(char *fmt, ...);
/* extern void ctrl_c(void); */
extern void cGets(char *buff);
extern BOOL kb_hit(void);
extern void unget_ch(char c);
extern int get_ch(void);


/* CRON.C */
extern void readcron(void);
extern int do_cron(int why_called);
extern int cando_event(int evnt);
extern void list_event(void);
extern void cron_commands(void);
extern void did_net(char *callnode);
extern void writeCrontab(void);
extern void readCrontab(void);


/* CTDL.C */
extern void main(int argc, char *argv[]);
extern char doRegular(char x, char c);
extern char getCommand(char *c, char prompt);
extern void parseArgs(int argc, char *argv[]);
extern void postEvent(char *fmt, ...);
extern void listEvents(void);

/* DO.C */
extern void doDownload(char ex);
extern void doUpload(char ex);
extern void doChat(char moreYet, char first);
extern void doEnter(char moreYet, char first);
extern void exclude(void);
extern void doGoto(char expand, char skip);
extern void doHelp(char expand);
extern void doIntro(void);
extern void doKnown(char moreYet, char first);
extern void doLogout(char expand, char first);
extern void doRead(char moreYet, char first);
extern void doXpert(void);
extern void doverbose(void);
extern void doNext(void);
extern void doPrevious(void);
extern void doNextHall(void);
extern void doPreviousHall(void);
extern void doList(char moreYet, char first);
extern void doSmallChat(void);

/* DOSYSOP.C */
extern char doSysop(void);
extern void do_SysopGroup(void);
extern void do_SysopHall(void);
extern void sysopunlink(void);
extern void globalverify(void);

/* DOAIDE.C */
extern void doAide(char moreYet, char first);
extern void msgNym(void);

/* DOWN.C */
extern void crashout(char *message);
extern void exitcitadel(void);

/* EDIT.C */
extern int editText(char *buf, int lim);
extern void putheader(BOOL first);
extern BOOL getText(void);
extern char *matchString(char *buf, char *pattern, char *bufEnd, char ver);
extern void replaceString(char *buf, int lim, char ver);
extern void wordcount(char *buf);
extern void fakeFullCase(char *text);
extern void xPutStr(FILE *fl, char *str);
extern void xPutLine(FILE *fl, const char *str, int len);
extern void GetFileMessage(FILE *fl, char *str, int mlen);
extern void cyclesignature(void);

/* FILES.C */
extern int ambig(char *filename);
extern int ambigUnlink(char *filename, char change);
extern void attributes(void);
extern void blocks(char *filename, int bsize);
extern long bytesfree(void);
extern int checkfilename(char *filename, char xtype);
extern int checkup(char *filename);
extern void filldirectory(char *filename);
extern void dir(char *filename);
extern double dltime(long size);
extern void entertextfile(void);
extern void enterwc(void);
extern void entrycopy(int element);
extern void entrymake(struct  find_t *file_buf);
extern unsigned char getattr(char far *filename);
extern void hide(char *filename);
extern void readdirectory(void);
extern void readtextfile(void);
extern void readwc(void);
extern void renamefile(void);
extern void setattr(char far *filename, unsigned char attr);
extern void strlower(char *string);
extern void textdown(char *filename);
extern void textup(char *filename);
extern void unlinkfile(void);
extern void wcdown(char *filename);
extern void wcup(char *filename);
extern void download(char c);
extern void upload(char c);
extern void upDownMnu(char cmd);
extern BOOL copyfile(char *source, char *dest);

/* FORMAT.C */
extern void sformat(char *str, char *fmt, char *val, ...);
extern void normalizeString(char *s);
extern int parse_it(char *words[], char input[]);
extern int qtext(char *buf, char *line, char end);
extern int strpos(char ch, char *str);
extern int substr(char *str1, char *str2);
extern int u_match(register char *s, register char *p);
extern void parseNetAddress(char *str, char *u, char *n, char *r, char *c);
extern char *deansi(char *str);
extern int  deansi_str_cmpi(char *string1, char *string2);
extern int  deansi_str_cmp(char *string1, char *string2);
extern void stripansi(char *string);

/* GROUP.C */
extern void cleargroupgen(void);
extern int groupseeshall(int hallslot);
extern int groupseesroom(int groupslot);
extern int pgroupseesroom(void);
extern int groupexists(char *groupname);
extern int ingroup(int groupslot);
extern void getgroup(void);
extern int partialgroup(char *groupname);

/* GRPEDIT.C */
extern void groupfunc(void);
extern void globalgroup(void);
extern void globaluser(void);
extern void killgroup(void);
extern void listgroup(void);
extern void newgroup(void);
extern void renamegroup(void);

/* HALL.C */
extern int accesshall(int slot);
extern void enterhall(void);
extern void gotodefaulthall(void);
extern int hallexists(char *hallname);
extern int iswindow(int roomslot);
extern void knownhalls(void);
extern int partialhall(char *hallname);
extern void readhalls(void);
extern int roominhall(int roomslot);
extern void stephall(int direction);

/* HALLEDIT.C */
extern void force(void);
extern void globalhall(void);
extern void hallfunc(void);
extern int xhallfunc(int roomslot, int hallslot, int xyn, int fl);
extern void killhall(void);
extern void listhalls(void);
extern void newhall(void);
extern void renamehall(void);
extern void windowfunc(void);
extern void moveHall(int offset);

/* HELP.C */
extern int dump(char *filename);
extern int dumpf(char *filename);
extern void tutorial(char *filename);
extern void hello(void);
extern void goodbye(void);
extern void nochat(BOOL reset);
extern void BLBRotate(char *base, char *ext, int *reg, int *ansi);
extern void usage(void);
/* extern void Help(BOOL expand); */

/* INFO.C */
extern void entercomment(char *filename, char *uploader, char *comment);
extern void setfileinfo(void);

/* INFOFILE.C */
extern void addinfo(char *filename, char *uploader, char *comment);
extern void entercomment(char *filename, char *uploader, char *comment);
extern void fillinfo(void);
extern int infoslot(char *filename);
extern void killinfo(char *filename);
extern void readinfofile(BOOL ask);
extern void setfileinfo(void);
extern void showinfo(void);
extern void updateinfo(void);
extern int batchinfo(int askuser);
extern void moveFile(void);

/* INIT.C */
extern void initCitadel(void);
extern void openFile(char *filename, FILE **fd);

/* INPUT.C */
extern void getNormStr(char *prompt, char *s, int size, char doEcho);
extern long getNumber(char *prompt, long bottom, long top, long dfaultnum);
extern void getString(char *prompt, char *buf, int lim, char QuestIsSpecial, char doEcho, char *dfault);
extern int getYesNo(char *prompt, char dfault);
extern int BBSCharReady(void);
extern void setio(char whichio, char echo, char outflag);
extern int iCharNE(void);
extern int iChar(void);
/* extern int  rchar2(unsigned  timeout,unsigned char  *ch); */

/* LIBRARY.C */
extern void getGroup(void);
extern void putGroup(void);
extern void getHall(void);
extern void putHall(void);
extern void getLog(struct logBuffer *lBuf, int n);
extern void putLog(struct logBuffer *lBuf, int n);
extern void getRoom(int rm);
extern void putRoom(int rm);
extern int readTables(void);
extern void writeTables(void);
extern void allocateTables(void);
extern int readMsgTab(void);
extern void writeMsgTab(void);
extern void checkdir(void);
extern void freeTables(void);

/* LOG.C */
extern int findPerson(char *name, struct logBuffer *lbuf);
extern int personexists(char *name);
extern void setlogconfig(void);
extern void setsysconfig(void);
extern void storeLog(void);
extern void displaypw(char *name, char *in, char *pw);
extern void normalizepw(char *InitPw, char *Initials, char *passWord);
extern void log2tab(struct lTable *lt, struct logBuffer *lb);

/* LOG2.C */
extern void forwardaddr(void);
extern void killuser(void);
extern void newPW(void);
extern void Readlog(BOOL revOrder);
extern void showuser(void);
extern void enterName(char *prompt, char *name, char *oldname);
extern void defaulthall(char *def);
extern void configure(BOOL new);
extern void showconfig(struct logBuffer *lbuf);

/* LOGEDIT.C */
extern void userEdit(void);
extern void userPointers(void);

/* LOGIN.C */
extern void login(void);
extern void setgroupgen(void);
extern void setroomgen(void);
extern void setlbvisit(void);
extern void slideLTab(int slot);
extern void minibin(void);
extern int pwslot(char *in, char *pw);
extern int pwexists(char *pw);
extern void doLogin(char moreYet);


/* LOGINNEW.C */
extern void loginNew(char *initials, char *password);
extern void newUser(char *initials, char *password);
extern void newUserFile(void);
extern int newlog(void);
extern int newslot(void);

/* LOGOUT.C */
extern void terminate(char discon);
extern void setalloldrooms(void);
extern void initroomgen(void);
extern void setdefaultconfig(void);

/* LZHFILE.C */
extern char lzhview(char *filename);
extern void readlzh(void);

/* MISC.C */
extern BOOL filexists(char *filename);
extern uint hash(char *str);
extern void hmemcpy(void huge *xto, void huge *xfrom, long size);
extern void h2memcpy(char huge *to, char huge *from, long size);
extern int changedir(char *path);
extern void changedisk(char disk);
extern char *ltoac(long num);
extern void editBorder(void);
extern void doBorder(void);
/* extern int l_distance(char *requested, char *found); */
extern int  long_LO(long);
extern int  long_HI(long);
extern long long_JOIN(int, int);

#ifdef NEWMSGTAB
extern char  int_LO(int);
extern char  int_HI(int);
extern int   int_JOIN(char, char);
#endif

extern int  bit_clear(char *bitbag, int bit_no);
extern int  bit_test(char *bitbag, int bit_no);
extern int  bit_set(char *bitbag, int bit_no);

/* MISC2.C */
extern void ringSystemREQ(void);
extern void dial_out(void);
extern void logo(void);
extern void greeting(void);
extern void systat(void);

/* MODEM.C */
extern int digitbaud(uint bigbaud);
extern void domcr(void);
extern void offhook(void);
extern void outstring(char *string);
extern void Mflush(void);
extern void Hangup(void);

/* MSG.C */
extern struct unkLst *addUnknownList(void); 
extern void clearmsgbuf(void);
extern BOOL mAbort(BOOL pause);
extern ulong indexslot(ulong msgno);
extern BOOL mayseemsg(void);
extern BOOL mayseeindexmsg(ulong slot);
extern ulong sizetable(void);
extern void indexmessage(ulong here);

/* MSGCFG.C */
extern void msgInit(void);
extern int zapMsgFile(void);
extern void slidemsgTab(ulong howmany);
/* extern void buildcopies(void); */

/* MSGMAKE.C */
extern BOOL makeMessage(void);
extern BOOL putMessage(void);
extern void aideMessage(void);
extern void specialMessage(void);
extern void putMsgChar(char c);
extern void notelogmessage(char *name);
extern void crunchmsgTab(ulong howmany);
extern void dPrintf(char *fmt, ...);
extern void overwrite(int bytes);
extern void putMsgStr(char *string);
extern void noteMessage(void);

/* MSGMOD.C */
extern void markroom(void);
extern void printmsglist(void);
extern void insertmsglist(void);
extern void killmsglist(void);
extern void sortmsglist(char direction);
extern int msglistsort(ulong **s1, ulong **s2);
extern int revmsglistsort(ulong **s1, ulong **s2);
extern void addtomsglist(ulong msgnumber);
extern void clearmsglist(void);
extern void automark(void);
extern void copymessage(ulong id, int roomno);
extern void deleteMessage(void);
extern void insert(void);
extern BOOL markIt(void);
extern void markmsg(void);
extern BOOL pullIt(void);
extern void changeheader(ulong id, int roomno, uchar attr);
/* extern void copyindex(int dest, int source); */

/* MSGREAD.C */
extern void printMessage(ulong id);
extern BOOL stepMessage(ulong *at, int dir);
extern void showMessages(char whichMess, char revOrder);
extern BOOL getMessage(void);
extern void getMsgStr(char *dest, int lim);
extern int getMsgChar(void);
extern void printheader(ulong id, ulong slot);

/* NET.C */
extern BOOL net_slave(void);
extern BOOL net_master(void);
extern BOOL slave(void);
extern BOOL master(void);
extern BOOL n_login(void);
extern BOOL wait_for(char *str);
extern BOOL net_callout(char *node);
extern void cleanup(void);
extern BOOL get_first_room(char *here, char *there);
extern BOOL get_next_room(char *here, char *there);
extern BOOL n_dial(void);

/* NETDC15.C */
extern BOOL dc15network(BOOL master);
extern void NametoId(char *string);
extern void IdtoName(char *string);
extern BOOL getId(char first, char *string);
extern int  IdExists(char *id);
extern BOOL autoroom(char *name);
extern BOOL checkidlist(char *id);

/* NETMAIL.C */
extern int NfindRoom(char *str);
extern BOOL alias(char *str);
extern BOOL route(char *str);
extern BOOL alias_route(char *str, char *srch);
extern BOOL save_mail(void);
extern BOOL region_route(char *reg, char *str);

/* NETMSG.C */
extern BOOL GetStr(FILE *fl, char *str, int mlen);
extern void GetFStr(FILE *fl, char *str, int mlen, int fLevel);
extern void PutStr(FILE *fl, char *str);
extern BOOL GetMessage(FILE *fl);
extern void PutMessage(FILE *fl);
extern void NewRoom(int room, char *filename);
extern void saveMessage(ulong id, FILE *fl);
extern int ReadMsgFl(int room, char *filename, char *here, char *there);
extern BOOL netcanseeroom(int roomslot);

/* NETNODE.C */
extern BOOL readnode(void);
extern BOOL getnode(char *nodename);

/* OUTPUT.C */
extern void mFormat(char *string);
extern void asciitable(void);
extern void doBS(void);
extern BOOL doCR(void);
extern void dospCR(void);
extern void doTAB(void);
extern void echocharacter(char c);
extern void oChar(register char c);
extern void updcrtpos(char c);
extern void mPrintf(char *fmt, ...);
extern void rmPrintf(char *fmt, ...);
extern void prtList(char *item);
extern int getWord(uchar *dest, register uchar *source, int offset, int lim);
extern void putWord(uchar *st);

/* PORT.C */
extern int ringdetect(void);
extern int MOReady(void);
extern int MIReady(void);
extern void Initport(void);
extern void pHangup(void);
extern void dumbmodemHangup(void);
extern int gotCarrier(void);
extern int getMod(void);
extern void raise_dtr(void);
extern void drop_dtr(void);
extern void baud(int baudrate);
extern void outMod(unsigned char ch);
extern void portInit(void);
extern void portExit(void);

/* ROOM.C */
extern int roomPosSlot(int room);
extern int canseeroom(int roomslot);
extern void dumpRoom(void);
extern void listRooms(unsigned int what, char numMess);
extern void RoomStatus(void);
extern int partialExist(char *roomname);
extern int partialExist_ignorehalls(char *roomname);
extern void printrm(int room);
extern void printroomVer(int room, char numMess);
extern char *makeRoomName(int room, char *string);
extern void roomdescription(void);
extern int roomExists(char *room);
extern void roomtalley(void);
extern void givePrompt(void);
extern void noteRoom(void);
extern void stepRoom(int direction);
extern void unGotoRoom(void);
extern int gotoRoom(char *roomname);

/* ROOM2.C */
extern int findRoom(void);
extern void formatSummary(char *buffer);
extern void makeRoom(void);
extern void killempties(void);
extern void killroom(void);
extern void massdelete(void);
extern void readbymsgno(void);
extern void moveRoom(int offset);

/* ROOMEDIT.C */
extern void renameRoom(void);

/* STRFTIME.C */
extern void strftime(char *outstr, int maxsize, char *formatstr, long tnow);

/* TERM.C */
extern void autoansi(void);
extern void localTermCap(char c);
/* extern void setTerm(char *t); */
/* extern void askTerm(void); */
extern void askAttributes(void); 
extern void setOutTrans(BOOL ibmAnsi);
extern void termCap(char c);
extern void setdefaultTerm(char t);
extern void setlogTerm(void);
extern char *attrtoansi(uchar attr, uchar normal);
extern void setdefaultcolors(void);

/* TIMEDATE.C */
extern void changeDate(void);
extern int dayofweek(void);
extern char *diffstamp(long oldtime);
extern void getdstamp(char *buffer, unsigned stamp);
extern void gettstamp(char *buffer, unsigned stamp);
extern int hour(void);
extern void set_date(void);
extern void pause(register int ptime);
extern void netpause(register int ptime);
extern void twirlypause(register int ptime);
extern int systimeout(time_t timer);

/* TRAP.C */
extern void trap(char *string, int what);
extern void SaveAideMess(void);
extern void amPrintf(char *fmt, ...);
extern void amZap(void);

/* VERSION.C */

/* WINDOW.C */
extern void clreol(void);
extern void cls(void);
extern void connectcls(void);
extern void cursoff(void);
extern void curson(void);
extern int gmode(void);
extern void help(void);
extern void altF10(void);
extern void position(uchar row, uchar column);
extern void clearline(unsigned int row, uchar attr);
extern void readpos(uchar *row, uchar *column);
extern void scroll(uchar row, uchar howmany, uchar attr);
extern void updatehelp(void);
extern void directstring(unsigned int row, char *str, uchar attr);
extern void directchar(char ch, uchar attr);
extern void biosstring(unsigned int row, char *str, uchar attr);
extern void bioschar(char ch, uchar attr);
extern char ansi(char c);
extern void save_screen(void);
extern void restore_screen(void);
extern void ScreenFree(void);
extern void updatealtF10(void);
extern void update25(void);
extern void setscreen(void);
extern void getScreenSize(uchar *Cols, uchar *Rows);
extern void readcursorsize(uchar *cursorstart, uchar *cursorend);

/* XMODEM.C */
extern int  xsend(char  *filename,int  mode);
extern int  xreceive(char  *filename,int  mode);
extern int  xget(FILE *fp);
extern int  xput(FILE *fp);
extern int  fillbuf(FILE *fp,char  _far *buf);
extern void  upsum(char  c);
extern int  rchar(unsigned  timeout,unsigned char  *ch);
extern void  status(char  *string,int  blocknum,int  errcount);
extern void  sleep(unsigned  delay);
extern void  complete(int  success,int  blocknum,int  errcount,FILE *fp);
extern void  cksend(char  ch);

/* ZIPFILE.C */
extern void readzip(void);
extern char zipview(char *filename);

