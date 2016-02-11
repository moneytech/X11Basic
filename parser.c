/* parser.c   Formelparser numerisch und Zeichenketten (c) Markus Hoffmann*/

/* This file is part of X11BASIC, the basic interpreter for Unix/X
 * ============================================================
 * X11BASIC is free software and comes with NO WARRANTY - read the file
 * COPYING for details
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "config.h"
#include "defs.h"
#include "globals.h"
#include "protos.h"
#include "array.h"
#include "functions.h"
#include "file.h"

#ifdef WINDOWS
#undef HAVE_LOGB
#undef HAVE_LOG1P
#undef HAVE_EXPM1
#undef HAVE_FORK
#endif

#ifdef __CYGWIN__  /* David Andersen  11.10.2003*/
#undef HAVE_LOGB
#endif


#ifndef HAVE_LOGB
double logb(double a) {return(log(a)/log(2));}
#endif
#ifndef HAVE_LOG1P
double log1p(double a) {return(log(1+a));}
#endif
#ifndef HAVE_EXPM1
double expm1(double a) {return(exp(a)-1);}
#endif

double f_nop(void *t) {return(0.0);}
STRING vs_error() {return(create_string("<ERROR>"));}
#ifndef NOGRAPHICS
int f_point(double v1, double v2) {  return(get_point((int)v1,(int)v2)); }
#endif
int f_bclr(double v1, double v2) { return((int)v1 & ~ (1 <<((int)v2))); }
int f_bset(double v1, double v2) { return((int)v1 | (1 <<((int)v2))); }
int f_bchg(double v1, double v2) { return((int)v1 ^ (1 <<((int)v2))); }
int f_btst(double v1, double v2) { return((((int)v1 & (1 <<((int)v2)))==0) ?  0 : -1); }
int f_shr(double v1, double v2) { return(((int)v1)>>((int)v2)); }
int f_shl(double v1, double v2) { return(((int)v1)<<((int)v2)); }

int f_instr(PARAMETER *,int);
int f_rinstr(PARAMETER *,int);
int f_glob(PARAMETER *,int);
int f_form_alert(PARAMETER *,int);
int f_form_center(PARAMETER *,int);
int f_form_dial(PARAMETER *,int);
int f_form_do(PARAMETER *,int);
int f_objc_draw(PARAMETER *,int);
int f_objc_offset(PARAMETER *,int);
int f_rsrc_gaddr(PARAMETER *,int);
int f_objc_find(PARAMETER *,int);
int f_get_color(PARAMETER *,int);

extern int f_symadr(PARAMETER *,int);
extern int f_exec(char *);
extern int shm_malloc(int,int);
extern int shm_attach(int);

int f_int(double b) {return((int)b);}
int f_fix(double b) {if(b>=0) return((int)b);
                      else return(-((int)(-b)));}
double f_pred(double b) {return(ceil(b-1));}
int f_succ(double b) {return((int)(b+1));}
int f_sgn(double b) {return(sgn(b));}
double f_frac(double b) {return(b-((double)((int)b)));}
int f_even(int b) {return(b&1 ? 0:-1);}
int f_odd(int b) {return(b&1 ? -1:0);}
int f_fak(int);
int f_random(double d) {return((int)((double)rand()/RAND_MAX*d));}
double f_rnd(double d) {return((double)rand()/RAND_MAX);}
int  f_srand(double d) {srand((int)d);return(0);}
double f_deg(double d) {return(d/PI*180);}
double f_rad(double d) {return(d*PI/180);}
int  f_dimf(char *pos) {return((double)do_dimension(variable_exist_type(pos)));}
int f_asc(STRING n) {  return((int)n.pointer[0]); }
int f_cvi(STRING n) {  return((int)(*((short *)n.pointer))); }
int f_cvl(STRING n) {  return((int)(*((long *)n.pointer))); }
double f_cvd(STRING n) {  return((double)(*((double *)n.pointer))); }
double f_cvf(STRING n) {  return((double)(*((float *)n.pointer))); }
double f_eval(STRING n) {return(parser(n.pointer));}

double f_add(double v1, double v2) {return(v1+v2);}
double f_sub(double v1, double v2) {return(v1-v2);}
double f_mul(double v1, double v2) {return(v1*v2);}
double f_div(double v1, double v2) {return(v1/v2);}

int f_len(STRING n)    { return(n.len); }
int f_exist(STRING n)  { return(-exist(n.pointer)); }
int f_size(STRING n)   { return(stat_size(n.pointer)); }
int f_nlink(STRING n)  { return(stat_nlink(n.pointer)); }
int f_device(STRING n) { return(stat_device(n.pointer)); }
int f_inode(STRING n)  { return(stat_inode(n.pointer)); }
int f_mode(STRING n)   { return(stat_mode(n.pointer)); }

double f_val(STRING n) { return((double)atof(n.pointer)); }
double f_ltextlen(STRING n) { return((double)ltextlen(ltextxfaktor,ltextpflg,n.pointer)); }

char *arrptr(char *);
#ifdef TINE
double f_tinemax(STRING n) { return(tinemax(n.pointer)); }
double f_tinemin(STRING n) { return(tinemin(n.pointer)); }
double f_tineget(STRING n) { return(tineget(n.pointer)); }
int f_tinesize(STRING n) { return(tinesize(n.pointer)); }
int f_tinetyp(STRING n) { return(tinetyp(n.pointer)); }
#endif
#ifdef DOOCS
double f_doocsget(STRING n) { return(doocsget(n.pointer)); }
int f_doocssize(STRING n) { return(doocssize(n.pointer)); }
int f_doocstyp(STRING n) { return(doocstyp(n.pointer)); }
double f_doocstimestamp(STRING n) { return(doocstimestamp(n.pointer)); }
#endif
#ifdef CONTROL
double f_csmax(STRING n) { return(csmax(n.pointer)); }
double f_csmin(STRING n) { return(csmin(n.pointer)); }
double f_csres(STRING n) { return(csres(n.pointer)); }
double f_csget(STRING n) { return(csget(n.pointer)); }
int f_cssize(STRING n) { return(cssize(n.pointer)); }
int f_cspid(STRING n) { return(cspid(n.pointer)); }
#endif
int f_malloc(int size) {return((int)malloc(size));}
int f_realloc(int adr,int size) {return((int)realloc((char *)adr,size));}
int f_peek(int adr) { return((int)(*(char *)adr));}
int f_dpeek(int adr) { return((int)(*(short *)adr));}
int f_lpeek(int adr) { return((int)(*(long *)adr));}
int f_lof(PARAMETER *plist,int e) {
  if(filenr[plist[0].integer]) return(lof(dptr[plist[0].integer]));
  else { xberror(24,""); return(0);} /* File nicht geoeffnet */
}
int f_loc(PARAMETER *plist,int e) {
  if(filenr[plist[0].integer]) return(ftell(dptr[plist[0].integer]));
  else { xberror(24,""); return(0);} /* File nicht geoeffnet */
}
int f_eof(PARAMETER *plist,int e) {
  if(plist[0].integer==-2) return((myeof(stdin))?-1:0);
  else if(filenr[plist[0].integer]) {
    fflush(dptr[plist[0].integer]);
    return(myeof(dptr[plist[0].integer])?-1:0);
  } else { xberror(24,""); return(0);} /* File nicht geoeffnet */
}

#ifndef NOGRAPHICS

int lsel_input(char *,STRING *,int,int);

int f_listselect(PARAMETER *plist,int e) {
  int sel=-1;
  if(e==3) sel=plist[2].integer;
  if(e>=2) {
    ARRAY a;
    a.pointer=plist[1].pointer;
    a.dimension=plist[1].integer;
    a.typ=plist[1].typ;
    return(lsel_input(plist[0].pointer,(STRING *)(a.pointer+a.dimension*INTSIZE),anz_eintraege(a),sel));
  }
}
#endif

const FUNCTION pfuncs[]= {  /* alphabetisch !!! */
 { F_ARGUMENT|F_DRET,  "!nulldummy", f_nop ,0,0   ,{0}},
 { F_DQUICK|F_DRET,    "ABS"       , fabs ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "ACOS"      , acos ,1,1     ,{PL_NUMBER}},
#ifndef WINDOWS
 { F_DQUICK|F_DRET,    "ACOSH"      , acosh ,1,1     ,{PL_NUMBER}},
#endif
 { F_DQUICK|F_DRET,    "ADD"     , f_add ,2,2     ,{PL_NUMBER,PL_NUMBER}},
 { F_ARGUMENT|F_IRET,  "ARRPTR"    , arrptr ,1,1     ,{PL_ARRAY}},
 { F_SQUICK|F_IRET,    "ASC"       , f_asc ,1,1   ,{PL_STRING}},
 { F_DQUICK|F_DRET,    "ASIN"      , asin ,1,1     ,{PL_NUMBER}},
#ifndef WINDOWS
 { F_DQUICK|F_DRET,    "ASINH"      , asinh ,1,1     ,{PL_NUMBER}},
#endif
 { F_DQUICK|F_DRET,    "ATAN"      , atan ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "ATAN2"     , atan2 ,2,2     ,{PL_NUMBER,PL_NUMBER}},
#ifndef WINDOWS
 { F_DQUICK|F_DRET,    "ATANH"     , atanh ,1,1     ,{PL_NUMBER}},
#endif
 { F_DQUICK|F_DRET,    "ATN"       , atan ,1,1     ,{PL_NUMBER}},

 { F_DQUICK|F_IRET,    "BCHG"      , f_bchg,2,2     ,{PL_NUMBER,PL_NUMBER }},
 { F_DQUICK|F_IRET,    "BCLR"      , f_bclr,2,2     ,{PL_NUMBER,PL_NUMBER }},
 { F_DQUICK|F_IRET,    "BSET"      , f_bset,2,2     ,{PL_NUMBER,PL_NUMBER }},
 { F_DQUICK|F_IRET,    "BTST"      , f_btst,2,2     ,{PL_NUMBER,PL_NUMBER }},

#ifndef WINDOWS
 { F_DQUICK|F_DRET,    "CBRT"      , cbrt ,1,1     ,{PL_NUMBER}},
#endif
 { F_DQUICK|F_DRET,    "CEIL"      , ceil ,1,1     ,{PL_NUMBER}},
 { F_PLISTE|F_IRET,    "COMBIN"    , f_combin ,2,2     ,{PL_INT,PL_INT}},
 { F_DQUICK|F_DRET,    "COS"       , cos ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "COSH"      , cosh ,1,1     ,{PL_NUMBER}},
 { F_PLISTE|F_IRET,    "CRC"       , f_crc ,1,2     ,{PL_STRING, PL_INT}},
#ifdef CONTROL
 { F_SQUICK|F_DRET,  "CSGET"     , f_csget ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_DRET,  "CSMAX"     , f_csmax ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_DRET,  "CSMIN"     , f_csmin ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_IRET,  "CSPID"     , f_cspid ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_DRET,  "CSRES"     , f_csres ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_IRET,  "CSSIZE"    , f_cssize ,1,1   ,{PL_STRING}},
#endif
 { F_SQUICK|F_DRET,  "CVD"       , f_cvd ,1,1     ,{PL_STRING}},
 { F_SQUICK|F_DRET,  "CVF"       , f_cvf ,1,1     ,{PL_STRING}},
 { F_SQUICK|F_IRET,  "CVI"       , f_cvi ,1,1     ,{PL_STRING}},
 { F_SQUICK|F_IRET,  "CVL"       , f_cvl ,1,1     ,{PL_STRING}},
 { F_SQUICK|F_IRET,  "CVS"       , f_cvf ,1,1     ,{PL_STRING}},

 { F_DQUICK|F_DRET,    "DEG"       , f_deg ,1,1     ,{PL_NUMBER}},
 { F_SQUICK|F_IRET,    "DEVICE"    , f_device,1,1   ,{PL_STRING}},
 { F_ARGUMENT|F_IRET,  "DIM?"      , f_dimf ,1,1      ,{PL_ARRAY}},
 { F_DQUICK|F_DRET,    "DIV"       , f_div ,2,2     ,{PL_NUMBER,PL_NUMBER}},
#ifdef DOOCS
 { F_SQUICK|F_DRET,  "DOOCSGET"     , f_doocsget ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_IRET,  "DOOCSSIZE"    , f_doocssize ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_DRET,  "DOOCSTIMESTAMP"    , f_doocstimestamp ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_IRET,  "DOOCSTYP"    , f_doocstyp ,1,1   ,{PL_STRING}},
#endif
 { F_IQUICK|F_IRET,    "DPEEK"     , f_dpeek ,1,1     ,{PL_INT}},

 { F_PLISTE|F_IRET,    "EOF"       , f_eof ,1,1     ,{PL_FILENR}},

 { F_SQUICK|F_DRET,  "EVAL"      , f_eval ,1,1      ,{PL_STRING}},
 { F_IQUICK|F_IRET,    "EVEN"       , f_even ,1,1     ,{PL_NUMBER}},
 { F_ARGUMENT|F_IRET,  "EXEC"       , f_exec ,1,2     ,{PL_NUMBER,PL_NUMBER}},
 { F_SQUICK|F_IRET,    "EXIST"      , f_exist ,1,1     ,{PL_STRING}},
 { F_DQUICK|F_DRET,    "EXP"       , exp ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "EXPM1"     , expm1 ,1,1     ,{PL_NUMBER}},

 { F_IQUICK|F_IRET,    "FACT"       , f_fak ,1,1     ,{PL_INT}},
 { F_DQUICK|F_IRET,    "FIX"       , f_fix ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "FLOOR"     , floor ,1,1     ,{PL_NUMBER}},
#ifdef HAVE_FORK
 { F_SIMPLE|F_IRET,    "FORK"     , fork ,0,0     },
#endif
#ifndef NOGRAPHICS
 { F_PLISTE|F_IRET,    "FORM_ALERT", f_form_alert ,2,2   ,{PL_INT,PL_STRING}},
 { F_PLISTE|F_IRET,    "FORM_CENTER", f_form_center ,5,5   ,{PL_INT,PL_NVAR,PL_NVAR,PL_NVAR,PL_NVAR}},
 { F_PLISTE|F_IRET,    "FORM_DIAL", f_form_dial ,9,9   ,{PL_INT,PL_INT,PL_INT,PL_INT,PL_INT,PL_INT,PL_INT,PL_INT,PL_INT}},
 { F_PLISTE|F_IRET,    "FORM_DO",   f_form_do ,1,1   ,{PL_INT}},
#endif
 { F_DQUICK|F_DRET,    "FRAC"      , f_frac ,1,1     ,{PL_NUMBER}},
 { F_SIMPLE|F_IRET,    "FREEFILE"  , f_freefile ,0,0  },

 { F_DQUICK|F_DRET,    "GASDEV"   , f_gasdev ,1,1     ,{PL_NUMBER}},
#ifndef NOGRAPHICS
 { F_PLISTE|F_IRET,    "GET_COLOR", f_get_color ,3,3   ,{PL_INT,PL_INT,PL_INT}},
#endif
 { F_PLISTE|F_IRET,    "GLOB"     , f_glob ,2,3   ,{PL_STRING,PL_STRING,PL_INT}},
 { F_IQUICK|F_IRET,    "GRAY"     , f_gray ,1,1     ,{PL_INT}},

 { F_DQUICK|F_DRET,    "HYPOT"     , hypot ,2,2     ,{PL_NUMBER,PL_NUMBER}},

 { F_SQUICK|F_IRET,    "INODE"     , f_inode,1,1   ,{PL_STRING}},
 { F_PLISTE|F_IRET,  "INP"       , inp8 ,1,1      ,{PL_FILENR}},
 { F_PLISTE|F_IRET,  "INP?"      , inpf ,1,1      ,{PL_FILENR}},
 { F_PLISTE|F_IRET,  "INP&"      , inp16 ,1,1      ,{PL_FILENR}},
 { F_PLISTE|F_IRET,  "INP%"      , inp32 ,1,1      ,{PL_FILENR}},
 { F_PLISTE|F_IRET,  "INSTR"     , f_instr ,2,3   ,{PL_STRING,PL_STRING,PL_INT}},

 { F_DQUICK|F_IRET,    "INT"       , f_int ,1,1     ,{PL_NUMBER}},
 { F_PLISTE|F_IRET,    "IOCTL"     , f_ioctl ,2,3     ,{PL_FILENR,PL_INT,PL_INT}},
 { F_SQUICK|F_IRET,    "JULIAN"    , f_julian ,1,1     ,{PL_STRING}},

 { F_SQUICK|F_IRET,    "LEN"       , f_len ,1,1   ,{PL_STRING}},
#ifndef NOGRAPHICS
 { F_PLISTE|F_IRET,    "LISTSELECT", f_listselect ,2,3   ,{PL_STRING,PL_SARRAY,PL_INT}},
#endif
 { F_DQUICK|F_DRET,    "LN"        , log ,1,1     ,{PL_NUMBER}},

 { F_PLISTE|F_IRET,    "LOC"       , f_loc ,1,1     ,{PL_FILENR}},
 { F_PLISTE|F_IRET,    "LOF"       , f_lof ,1,1     ,{PL_FILENR}},

 { F_DQUICK|F_DRET,    "LOG"       , log ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "LOG10"     , log10 ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "LOG1P"     , log1p ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "LOGB"      , logb  ,1,1     ,{PL_NUMBER}},
 { F_IQUICK|F_IRET,    "LPEEK"    , f_lpeek ,1,1     ,{PL_INT}},
 { F_SQUICK|F_DRET,    "LTEXTLEN"  , f_ltextlen ,1,1   ,{PL_STRING}},

 { F_IQUICK|F_IRET,    "MALLOC"    , f_malloc ,1,1     ,{PL_INT}},
 { F_PLISTE|F_DRET,    "MAX"     , f_max ,1,-1     ,{PL_NUMBER,PL_NUMBER,PL_NUMBER}},
 { F_PLISTE|F_DRET,    "MIN"     , f_min ,1,-1     ,{PL_NUMBER,PL_NUMBER,PL_NUMBER}},
 { F_DQUICK|F_DRET,    "MOD"     , fmod ,2,2     ,{PL_NUMBER,PL_NUMBER }},
 { F_SQUICK|F_IRET,    "MODE"     , f_mode,1,1   ,{PL_STRING}},
 { F_DQUICK|F_DRET,    "MUL"     , f_mul ,2,2     ,{PL_NUMBER,PL_NUMBER}},

 { F_SQUICK|F_IRET,    "NLINK"     , f_nlink,1,1   ,{PL_STRING}},

#ifndef NOGRAPHICS
 { F_PLISTE|F_IRET,    "OBJC_DRAW", f_objc_draw ,5,5   ,{PL_INT,PL_INT,PL_INT,PL_INT,PL_INT}},
 { F_PLISTE|F_IRET,    "OBJC_FIND", f_objc_find ,3,3   ,{PL_INT,PL_INT,PL_INT}},
 { F_PLISTE|F_IRET,    "OBJC_OFFSET", f_objc_offset ,4,4,{PL_INT,PL_INT,PL_NVAR,PL_NVAR}},
#endif
 { F_IQUICK|F_IRET,    "ODD"       , f_odd ,1,1     ,{PL_NUMBER}},

 { F_IQUICK|F_IRET,    "PEEK"      , f_peek ,1,1     ,{PL_INT}},
#ifndef NOGRAPHICS
 { F_DQUICK|F_IRET,    "POINT"     , f_point ,2,2     ,{PL_NUMBER, PL_NUMBER }},
#endif
 { F_DQUICK|F_DRET,    "PRED"      , f_pred ,1,1     ,{PL_NUMBER}},
#ifndef NOGRAPHICS
 { F_DQUICK|F_IRET,    "PTST"     , f_point ,2,2     ,{PL_NUMBER, PL_NUMBER }},
#endif

 { F_DQUICK|F_DRET,    "RAD"      , f_rad ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_IRET,    "RAND"      , rand ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_IRET,    "RANDOM"    , f_random ,1,1     ,{PL_NUMBER}},
 { F_IQUICK|F_IRET,    "REALLOC"    , f_realloc ,2,2     ,{PL_INT,PL_INT}},
 { F_PLISTE|F_IRET,    "RINSTR"    , f_rinstr ,2,3  ,{PL_STRING,PL_STRING,PL_INT}},
 { F_DQUICK|F_DRET,    "RND"       , f_rnd ,1,1     ,{PL_NUMBER}},
 { F_PLISTE|F_DRET,    "ROUND"     , f_round ,1,2   ,{PL_NUMBER,PL_INT}},
#ifndef NOGRAPHICS
 { F_PLISTE|F_IRET,    "RSRC_GADDR", f_rsrc_gaddr ,2,2   ,{PL_INT,PL_INT}},
#endif

 { F_DQUICK|F_IRET,    "SGN"       , f_sgn ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_IRET,    "SHL"      , f_shl,2,2     ,{PL_NUMBER,PL_NUMBER}},
 { F_IQUICK|F_IRET,    "SHM_ATTACH"    , shm_attach ,1,1     ,{PL_INT}},
 { F_IQUICK|F_IRET,    "SHM_MALLOC"    , shm_malloc ,2,2     ,{PL_INT,PL_INT}},
 { F_DQUICK|F_IRET,    "SHR"      , f_shr,2,2     ,{PL_NUMBER,PL_NUMBER}},
 { F_DQUICK|F_DRET,    "SIN"       , sin ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "SINH"      , sinh ,1,1     ,{PL_NUMBER}},
 { F_SQUICK|F_IRET,    "SIZE"     , f_size,1,1   ,{PL_STRING}},
 { F_DQUICK|F_DRET,    "SQR"       , sqrt ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "SQRT"      , sqrt ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_IRET,    "SRAND"     , f_srand ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "SUB"     , f_sub ,2,2     ,{PL_NUMBER,PL_NUMBER}},
 { F_DQUICK|F_IRET,    "SUCC"      , f_succ ,1,1     ,{PL_NUMBER}},
 { F_PLISTE|F_IRET,    "SYM_ADR"   , f_symadr ,2,2   ,{PL_FILENR,PL_STRING}},

 { F_DQUICK|F_DRET,    "TAN"       , tan ,1,1     ,{PL_NUMBER}},
 { F_DQUICK|F_DRET,    "TANH"       , tanh ,1,1     ,{PL_NUMBER}},
#ifdef TINE
 { F_SQUICK|F_DRET,  "TINEGET"     , f_tineget ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_DRET,  "TINEMAX"     , f_tinemax ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_DRET,  "TINEMIN"     , f_tinemin ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_IRET,  "TINESIZE"    , f_tinesize ,1,1   ,{PL_STRING}},
 { F_SQUICK|F_IRET,  "TINETYP"    , f_tinetyp ,1,1   ,{PL_STRING}},
#endif
 { F_DQUICK|F_DRET,    "TRUNC"     , f_fix ,1,1     ,{PL_NUMBER}},
 { F_ARGUMENT|F_IRET,  "TYP?"       , type2 ,1,1     ,{PL_ALL}},

 { F_SQUICK|F_DRET,  "VAL"       , f_val ,1,1     ,{PL_STRING}},
 { F_ARGUMENT|F_IRET,  "VARPTR"    , varptr ,1,1     ,{PL_ALL}},

 { F_ARGUMENT|F_IRET,  "WORT_SEP"    , do_wort_sep ,3,-1     ,{PL_STRING,0}},

};
const int anzpfuncs=sizeof(pfuncs)/sizeof(FUNCTION);


//  This code performs an order-0 adaptive arithmetic decoding
//  function on an input file/stream, and sends the result to an
//  output file or stream.
//
//  This program contains the source code from the 1987 CACM
//  article by Witten, Neal, and Cleary.  I have taken the
//  source modules and combined them into this single file for
//  ease of distribution and compilation.  Other than that,
//  the code is essentially unchanged.
#define No_of_chars 256                 /* Number of character symbols      */
#define EOF_symbol (No_of_chars+1)      /* Index of EOF symbol              */

#define No_of_symbols (No_of_chars+1)   /* Total number of symbols          */

int char_to_index[No_of_chars];         /* To index from character          */
unsigned char index_to_char[No_of_symbols+1]; /* To character from index    */
typedef long code_value;                /* Type of an arithmetic code value */

static code_value value;        /* Currently-seen code value                */
static code_value low, high;    /* Ends of current code region              */
int freq[No_of_symbols+1];      /* Symbol frequencies                       */
int cum_freq[No_of_symbols+1];  /* Cumulative symbol frequencies            */
#define Max_frequency 16383             /* Maximum allowed frequency count  */
#define Code_value_bits 16              /* Number of bits in a code value   */
#define Top_value (((long)1<<Code_value_bits)-1)      /* Largest code value */
#define First_qtr (Top_value/4+1)       /* Point after first quarter        */
#define Half      (2*First_qtr)         /* Point after first half           */
#define Third_qtr (3*First_qtr)         /* Point after third quarter        */
int buffer;                     /* Bits waiting to be input                 */
int bits_to_go;                 /* Number of bits still in buffer           */
int garbage_bits;               /* Number of bits past end-of-file          */
long bits_to_follow;            /* Number of opposite bits to output after  */


unsigned char *put_pointer;
int put_size;

inline int input_bit() {
  int t;
    if (bits_to_go==0) {                        /* Read the next byte if no */
        if(put_size>0) {
          buffer = *put_pointer&0xff;                   /* bits are left in buffer. */
          put_pointer++;
          put_size--;
        } else {
	  buffer=0;
            garbage_bits += 1;                      /* Return arbitrary bits*/
            if (garbage_bits>Code_value_bits-2) {   /* after eof, but check */
                printf("ARID: Bad input!\n"); /* for too many such.   */

            }
        }
        bits_to_go = 8;
    }
    t = buffer&1;                               /* Return the next bit from */
    buffer >>= 1;                               /* the bottom of the byte.  */
    bits_to_go--;
    return t;
}
inline void output_bit(int bit){
  buffer>>=1;
  if (bit) buffer |= 0x80;
    bits_to_go--;
    if (bits_to_go==0) {
        *put_pointer=buffer;
	put_pointer++;
        put_size++;
        bits_to_go = 8;
    }
}
void bit_plus_follow( int bit ){
  output_bit(bit);                            /* Output the bit.          */
    while (bits_to_follow>0) {
        output_bit(!bit);                       /* Output bits_to_follow    */
        bits_to_follow -= 1;                    /* opposite bits. Set       */
    }                                           /* bits_to_follow to zero.  */
}

void encode_symbol(int symbol,int cum_freq[] )
{   long range;                 /* Size of the current code region          */
    range = (long)(high-low)+1;
    high = low +                                /* Narrow the code region   */
      (range*cum_freq[symbol-1])/cum_freq[0]-1; /* to that allotted to this */
    low = low +                                 /* symbol.                  */
      (range*cum_freq[symbol])/cum_freq[0];
    for (;;) {                                  /* Loop to output bits.     */
        if (high<Half) {
            bit_plus_follow(0);                 /* Output 0 if in low half. */
        }
        else if (low>=Half) {                   /* Output 1 if in high half.*/
            bit_plus_follow(1);
            low -= Half;
            high -= Half;                       /* Subtract offset to top.  */
        }
        else if (low>=First_qtr                 /* Output an opposite bit   */
              && high<Third_qtr) {              /* later if in middle half. */
            bits_to_follow += 1;
            low -= First_qtr;                   /* Subtract offset to middle*/
            high -= First_qtr;
        }
        else break;                             /* Otherwise exit loop.     */
        low = 2*low;
        high = 2*high+1;                        /* Scale up code range.     */
    }
}

int decode_symbol( int cum_freq[] )
{   long range;                 /* Size of current code region              */
    int cum;                    /* Cumulative frequency calculated          */
    int symbol;                 /* Symbol decoded                           */
    range = (long)(high-low)+1;
    cum = (int)                                 /* Find cum freq for value. */
      ((((long)(value-low)+1)*cum_freq[0]-1)/range);
    for (symbol = 1; cum_freq[symbol]>cum; symbol++) ; /* Then find symbol. */
    high = low +                                /* Narrow the code region   */
      (range*cum_freq[symbol-1])/cum_freq[0]-1; /* to that allotted to this */
    low = low +                                 /* symbol.                  */
      (range*cum_freq[symbol])/cum_freq[0];
    for (;;) {                                  /* Loop to get rid of bits. */
        if (high<Half) {
            /* nothing */                       /* Expand low half.         */
        }
        else if (low>=Half) {                   /* Expand high half.        */
            value -= Half;
            low -= Half;                        /* Subtract offset to top.  */
            high -= Half;
        }
        else if (low>=First_qtr                 /* Expand middle half.      */
              && high<Third_qtr) {
            value -= First_qtr;
            low -= First_qtr;                   /* Subtract offset to middle*/
            high -= First_qtr;
        }
        else break;                             /* Otherwise exit loop.     */
        low = 2*low;
        high = 2*high+1;                        /* Scale up code range.     */
        value = 2*value+input_bit();            /* Move in next input bit.  */
    }
    return symbol;
}


void start_model(){
  int i;
    for(i=0;i<No_of_chars;i++) {           /* Set up tables that       */
        char_to_index[i]=i+1;                 /* translate between symbol */
        index_to_char[i+1]=(unsigned char)i; /* indexes and characters.  */
    }
    for (i=0;i<=No_of_symbols;i++) {        /* Set up initial frequency */
        freq[i]=1;                            /* counts to be one for all */
        cum_freq[i]=No_of_symbols-i;          /* symbols.                 */
    }
    *freq=0;                                /* Freq[0] must not be the  */
}                                               /* same as freq[1].         */

void update_model(int symbol){
  int i;                      /* New index for symbol                     */
    if (cum_freq[0]==Max_frequency) {           /* See if frequency counts  */
        int cum;                                /* are at their maximum.    */
        cum = 0;
        for (i = No_of_symbols; i>=0; i--) {    /* If so, halve all the     */
            freq[i] = (freq[i]+1)/2;            /* counts (keeping them     */
            cum_freq[i] = cum;                  /* non-zero).               */
            cum += freq[i];
        }
    }
    for (i = symbol; freq[i]==freq[i-1]; i--) ; /* Find symbol's new index. */
    if (i<symbol) {
        int ch_i, ch_symbol;
        ch_i = index_to_char[i];                /* Update the translation   */
        ch_symbol = index_to_char[symbol];      /* tables if the symbol has */
        index_to_char[i] = (unsigned char) ch_symbol; /* moved.             */
        index_to_char[symbol] = (unsigned char) ch_i;
        char_to_index[ch_i] = symbol;
        char_to_index[ch_symbol] = i;
    }
    freq[i] += 1;                               /* Increment the frequency  */
    while (i>0) {                               /* count for the symbol and */
        i -= 1;                                 /* update the cumulative    */
        cum_freq[i] += 1;                       /* frequencies.             */
    }
}

STRING f_aries(STRING n) {  /* order-0 adaptive arithmetic encoding */
  STRING ergebnis;
  int i;
  int size=n.len;
  ergebnis.pointer=malloc(size+1);
  put_pointer=ergebnis.pointer;
  put_size=0;

  start_model();
  /* start_outputing_bits */
  buffer=0;
  bits_to_go=8;
  /* Start encoding  */
  low=0;
  high=Top_value;
  bits_to_follow=0;
    for (i=0;i<n.len;i++) {
        int ch; int symbol;
        ch=n.pointer[i]&0xff;
        symbol=char_to_index[ch];
        encode_symbol(symbol,cum_freq);
        update_model(symbol);
    }
    encode_symbol(EOF_symbol,cum_freq);
    /* done_encoding */
    bits_to_follow += 1;                        /* Output two bits that     */
    if (low<First_qtr) bit_plus_follow(0);      /* select the quarter that  */
    else bit_plus_follow(1);                    /* the current code range   */                                               /* contains.                */

  /* done_outputing_bits   */
  *put_pointer++=(buffer>>bits_to_go);
  put_size++;
  ergebnis.len=put_size;
  return(ergebnis);
}
STRING f_arids(STRING n) {  /* order-0 adaptive arithmetic decoding */
  STRING ergebnis;
  int j=0,i;
  int size=n.len;
  put_pointer=n.pointer;
  put_size=n.len;
  ergebnis.pointer=malloc(size+1);

  start_model();                              /* Set up other modules.    */

  /* start_inputing_bits */
  bits_to_go=0;
  garbage_bits=0;
  /* start_decoding   */
  value = 0;
  for(i=1;i<=Code_value_bits;i++) value=2*value+input_bit();
  low=0;
  high=Top_value;


    for (;;) {
        int ch; int symbol;
        symbol = decode_symbol(cum_freq);
        if (symbol==EOF_symbol) break;
        ch = index_to_char[symbol];

        ergebnis.pointer[j++]=ch;
        if(j>=size) {
	  size=2*(size+1);
	  ergebnis.pointer=realloc(ergebnis.pointer,size+1);
	}
        update_model(symbol);
    }
  ergebnis.len=j;
  return(ergebnis);
}

STRING f_compresss(STRING n) {
  STRING ergebnis,a,b;
  a=f_rles(n);
  b=f_bwtes(a);
  free(a.pointer);
  a=f_mtfes(b);
  free(b.pointer);
  b=f_rles(a);
  free(a.pointer);
  ergebnis=f_aries(b);
  free(b.pointer);
  return(ergebnis);
}
STRING f_uncompresss(STRING n) {
  STRING ergebnis,a,b;
  b=f_arids(n);
  a=f_rlds(b);
  free(b.pointer);
  b=f_mtfds(a);
  free(a.pointer);
  a=f_bwtds(b);
  free(b.pointer);
  ergebnis=f_rlds(a);
  free(a.pointer);
  return(ergebnis);
}
STRING f_trims(STRING n) {
  STRING ergebnis;
  ergebnis.pointer=malloc(n.len+1);
  xtrim(n.pointer,0,ergebnis.pointer);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}

STRING f_xtrims(STRING n) {
  STRING ergebnis;
  ergebnis.pointer=malloc(n.len+1);
  xtrim(n.pointer,1,ergebnis.pointer);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_envs(STRING n) {return(create_string(getenv(n.pointer)));}
#ifdef CONTROL
STRING f_csgets(STRING n) {
  STRING ergebnis;
  ergebnis.pointer=csgets(n.pointer);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_csunits(STRING n) {
  STRING ergebnis;
  ergebnis.pointer=csunit(n.pointer);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_cspnames(int n) {
  STRING ergebnis;
  ergebnis.pointer=cspname(n);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
#endif
#ifdef TINE
STRING f_tinegets(STRING n) { return(tinegets(n.pointer)); }

STRING f_tineunits(STRING n) {
  STRING ergebnis;
  ergebnis.pointer=tineunit(n.pointer);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_tineinfos(STRING n) {
  STRING ergebnis;
  ergebnis.pointer=tineinfo(n.pointer);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_tinequerys(PARAMETER *plist,int e) {
  if(e>=2) return(tinequery(plist[0].pointer,plist[1].integer));
  else return(create_string(NULL));
}
#endif
#ifdef DOOCS
STRING f_doocsgets(STRING n) { return(doocsgets(n.pointer)); }
STRING f_doocsinfos(STRING n) { return(doocsinfos(n.pointer)); }
#endif
STRING f_terminalnames(char *n) {
  STRING ergebnis;
  int i=get_number(n);
  if(filenr[i]) ergebnis.pointer=terminalname(fileno(dptr[i]));
  else {
    xberror(24,""); /* File nicht geoeffnet */
    return(vs_error());
  }
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_systems(STRING n) {
  STRING ergebnis;
  FILE *dptr=popen(n.pointer,"r");

  if (dptr==NULL) {
    io_error(errno,"popen");
    ergebnis.pointer=malloc(38+n.len);
    sprintf(ergebnis.pointer,"couldn't execute '%s'. errno=%d",n.pointer,errno);
  } else {
    int len=0;
    int limit=1024;
    int c;
    ergebnis.pointer=NULL;
    do {
      ergebnis.pointer=realloc(ergebnis.pointer,limit);
     /* printf("Bufferlaenge: %d Bytes.\n",limit); */
      while(len<limit) {
        c=fgetc(dptr);
        if(c==EOF) {ergebnis.pointer[len]='\0';break;}
        ergebnis.pointer[len++]=(char)c;
      }
      limit+=len;
    } while(c!=EOF);
    if(pclose(dptr)==-1) io_error(errno,"pclose");
  }
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_errs(int n) { return(create_string(error_text((char)n,NULL))); }
STRING f_prgs(int n) {
  if(n>=prglen || n<0) xberror(16,"PRG$"); /* Feldindex zu gross*/
  return(create_string(program[min(prglen-1,max(n,0))]));
}
STRING f_params(int n) {
  if(n>=param_anzahl || n<0) return(create_string(NULL));
  else return(create_string(param_argumente[min(param_anzahl-1,max(n,0))]));
}
STRING f_unixdates(int n) {
  STRING ergebnis;
  struct tm * loctim;
  loctim=localtime((time_t *)(&n));
  ergebnis.pointer=malloc(12);
  sprintf(ergebnis.pointer,"%02d.%02d.%04d",loctim->tm_mday,loctim->tm_mon+1,1900+loctim->tm_year);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_unixtimes(int n) {
  STRING ergebnis;
  struct tm * loctim;
  loctim=localtime((time_t *)&n);
  ergebnis.pointer=malloc(16);
  sprintf(ergebnis.pointer,"%02d:%02d:%02d",loctim->tm_hour,loctim->tm_min,loctim->tm_sec);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_strs(PARAMETER *plist,int e) {         /* STR$(a[,b[,c[,d]]])     */
  STRING ergebnis;
  int b=-1,c=13,mode=0;
  char formatter[24];
  ergebnis.pointer=malloc(64);
  if(e>1) b=min(50,max(0,plist[1].integer));
  if(e>2) c=min(50,max(0,plist[2].integer));
  if(e>3) mode=plist[3].integer;
  if(mode==0 && b!=-1) sprintf(formatter,"%%%d.%dg",b,c);
  else if (mode==1 && b!=-1) sprintf(formatter,"%%0%d.%dg",b,c);
  else  sprintf(formatter,"%%.13g");
  sprintf(ergebnis.pointer,formatter,plist[0].real);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING f_bins(PARAMETER *plist,int e) {
  STRING ergebnis;
  unsigned int a=plist[0].integer;
  int j,b=8,i=0;
  if(e==2) b=max(0,plist[1].integer);
  ergebnis.pointer=malloc(b+1);
  for(j=b;j>0;j--) ergebnis.pointer[i++]=((a&(1<<(j-1)))  ? '1':'0');
  ergebnis.pointer[i]=0;
  ergebnis.len=i;
  return(ergebnis);
}
STRING f_hexs(PARAMETER *plist, int e) {return(hexoct_to_string('x',plist,e));}
STRING f_octs(PARAMETER *plist, int e) {return(hexoct_to_string('o',plist,e));}

/* Systemvariablen vom typ String */

int v_false() {return(0);}
int v_true() {return(-1);}
int v_err() { extern int globalerr; return(globalerr);}
int v_ccserr() {return(ccs_err);}
#ifdef CONTROL
int v_ccsaplid() {return(aplid);}
#endif
int v_sp() {return(sp);}
int v_pc() {return(pc);}
double v_timer() {
#ifdef WINDOWS
#if 0
       return((double)GetTickCount()/1000.0);
#else
       return((double)clock()/CLOCKS_PER_SEC);
#endif
#else
        struct timespec t;
	struct {
               int  tz_minuteswest; /* minutes W of Greenwich */
               int  tz_dsttime;     /* type of dst correction */
       } tz;
	gettimeofday(&t,&tz);
	return((double)t.tv_sec+(double)t.tv_nsec/1000000);
#endif
}
int v_stimer() {   /* Sekunden-Timer */
  time_t timec=time(NULL);
  if(timec==-1) io_error(errno,"TIMER");
  return(timec);
}
extern int getrowcols(int *, int *);

int v_cols() {   /* Anzahl Spalten des Terminals */
  int rows=0,cols=0;
  getrowcols(&rows,&cols);
  return(cols);
}
int v_rows() {   /* Anzahl Zeilen des Terminals */
  int rows=0,cols=0;
  getrowcols(&rows,&cols);
  return(rows);
}

double v_ctimer() {return((double)clock()/CLOCKS_PER_SEC);}
double v_pi() {return(PI);}
extern int mousex(),mousey(), mousek(), mouses();
const SYSVAR sysvars[]= {  /* alphabetisch !!! */
 { PL_LEER,   "!nulldummy", v_false},
#ifdef CONTROL
 { PL_INT,    "CCSAPLID",   v_ccsaplid},
#endif
 { PL_INT,    "CCSERR",     v_ccserr},
 { PL_INT,    "COLS",       v_cols},
 { PL_FLOAT,  "CTIMER",     v_ctimer},
 { PL_INT,    "ERR",        v_err},
 { PL_INT,    "FALSE",      v_false},
#ifndef NOGRAPHICS
 { PL_INT,    "MOUSEK",     mousek},
 { PL_INT,    "MOUSES",     mouses},
 { PL_INT,    "MOUSEX",     mousex},
 { PL_INT,    "MOUSEY",     mousey},
#endif
 { PL_INT,    "PC",         v_pc},
 { PL_FLOAT,  "PI",         v_pi},
 { PL_INT,    "ROWS",       v_rows},
 { PL_INT,    "SP",         v_sp},
 { PL_INT,    "STIMER",     v_stimer},
 { PL_FLOAT,  "TIMER",      v_timer},
 { PL_INT,    "TRUE",       v_true},
#ifdef WINDOWS
 { PL_INT,    "WIN32?",     v_true},
#endif
#ifndef WINDOWS
 { PL_INT,    "UNIX?",      v_true},
#endif
};
const int anzsysvars=sizeof(sysvars)/sizeof(SYSVAR);

STRING vs_date() {
  STRING ergebnis;
  time_t timec;
  struct tm * loctim;
  timec = time(&timec);
  loctim=localtime(&timec);
  ergebnis.pointer=malloc(12);
  sprintf(ergebnis.pointer,"%02d.%02d.%04d",loctim->tm_mday,loctim->tm_mon+1,1900+loctim->tm_year);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING vs_time() {
  STRING ergebnis;
  time_t timec;
  struct tm * loctim;
  timec = time(&timec);
  loctim=localtime(&timec);
  ergebnis.pointer=malloc(9);
  strncpy(ergebnis.pointer,ctime(&timec)+11,8);
  ergebnis.len=8;
  return(ergebnis);
}
STRING vs_trace() {
  if(pc>=0 && pc<prglen) {
    STRING ergebnis;
    ergebnis.pointer=malloc(strlen(program[pc])+1);
    strcpy(ergebnis.pointer,program[pc]);
    xtrim(ergebnis.pointer,TRUE,ergebnis.pointer);
    ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
  } else return(vs_error());
}
STRING vs_terminalname() {
  STRING ergebnis;
  ergebnis.pointer=terminalname(STDIN_FILENO);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}
STRING vs_inkey() {  return(create_string(inkey())); }

const SYSSVAR syssvars[]= {  /* alphabetisch !!! */
 { PL_LEER,   "!nulldummy", vs_error},
 { PL_STRING, "DATE$", vs_date},
 { PL_STRING, "INKEY$", vs_inkey},
 { PL_STRING, "TERMINALNAME$", vs_terminalname},
 { PL_STRING, "TIME$", vs_time},
 { PL_STRING, "TRACE$", vs_trace},
};
const int anzsyssvars=sizeof(syssvars)/sizeof(SYSSVAR);

const SFUNCTION psfuncs[]= {  /* alphabetisch !!! */

 { F_ARGUMENT,  "!nulldummy", f_nop ,0,0   ,{0}},
 { F_SQUICK,    "ARID$"  , f_arids ,1,1   ,{PL_STRING}},
 { F_SQUICK,    "ARIE$"  , f_aries ,1,1   ,{PL_STRING}},
 { F_PLISTE,  "BIN$"    , f_bins ,1,2   ,{PL_INT,PL_INT}},
 { F_SQUICK,    "BWTD$"  , f_bwtds ,1,1   ,{PL_STRING}},
 { F_SQUICK,    "BWTE$"  , f_bwtes ,1,1   ,{PL_STRING}},

 { F_IQUICK,    "CHR$"    , f_chrs ,1,1   ,{PL_NUMBER}},
 { F_SQUICK,    "COMPRESS$"    , f_compresss ,1,1   ,{PL_STRING}},
#ifdef CONTROL
 { F_SQUICK,    "CSGET$"    , f_csgets ,1,1   ,{PL_STRING}},
 { F_IQUICK,    "CSPNAME$"  , f_cspnames ,1,1   ,{PL_INT}},
 { F_SQUICK,    "CSUNIT$"   , f_csunits ,1,1   ,{PL_STRING}},
#endif
 { F_PLISTE,    "DECRYPT$", f_decrypts ,2,2   ,{PL_STRING,PL_STRING}},
#ifdef DOOCS
 { F_SQUICK,    "DOOCSGET$"    , f_doocsgets ,1,1   ,{PL_STRING}},
 { F_SQUICK,    "DOOCSINFO$"    , f_doocsinfos ,1,1   ,{PL_STRING}},
#endif

 { F_PLISTE,    "ENCRYPT$", f_encrypts ,2,2   ,{PL_STRING,PL_STRING}},
 { F_SQUICK,    "ENV$"    , f_envs ,1,1   ,{PL_STRING}},
 { F_IQUICK,    "ERR$"    , f_errs ,1,1   ,{PL_NUMBER}},
 { F_PLISTE,  "HEX$"    , f_hexs ,1,4   ,{PL_INT,PL_INT,PL_INT,PL_INT}},
 { F_SQUICK,    "INLINE$" , f_inlines ,1,1   ,{PL_STRING}},
 { F_ARGUMENT,  "INPUT$"  , f_inputs ,1,2   ,{PL_FILENR,PL_NUMBER}},
 { F_IQUICK,    "JULDATE$" , f_juldates ,1,1   ,{PL_INT}},

 { F_PLISTE,    "LEFT$" , f_lefts ,1,2   ,{PL_STRING,PL_INT}},
 { F_ARGUMENT,  "LINEINPUT$" , f_lineinputs ,1,1   ,{PL_FILENR}},
 { F_SQUICK,    "LOWER$"    , f_lowers ,1,1   ,{PL_STRING}},

 { F_PLISTE,    "MID$"    , f_mids ,2,3   ,{PL_STRING,PL_INT,PL_INT}},
 { F_AQUICK,    "MKA$"    , array_to_string ,1,1   ,{PL_ARRAY}},
 { F_DQUICK,    "MKD$"    , f_mkds ,1,1   ,{PL_NUMBER}},
 { F_DQUICK,    "MKF$"    , f_mkfs ,1,1   ,{PL_NUMBER}},
 { F_IQUICK,    "MKI$"    , f_mkis ,1,1   ,{PL_INT}},
 { F_IQUICK,    "MKL$"    , f_mkls ,1,1   ,{PL_INT}},
 { F_DQUICK,    "MKS$"    , f_mkfs ,1,1   ,{PL_NUMBER}},
 { F_SQUICK,    "MTFD$"  , f_mtfds ,1,1   ,{PL_STRING}},
 { F_SQUICK,    "MTFE$"  , f_mtfes ,1,1   ,{PL_STRING}},

 { F_PLISTE,  "OCT$"    , f_octs ,1,4   ,{PL_INT,PL_INT,PL_INT,PL_INT}},

 { F_IQUICK,    "PARAM$"  , f_params ,1,1   ,{PL_INT}},
 { F_IQUICK,    "PRG$"    , f_prgs ,1,1   ,{PL_INT}},
 { F_PLISTE,    "REPLACE$"  , f_replaces ,3,3   ,{PL_STRING,PL_STRING,PL_STRING}},
 { F_SQUICK,    "REVERSE$"  , f_reverses ,1,1   ,{PL_STRING}},
 { F_PLISTE,    "RIGHT$"  , f_rights ,1,2   ,{PL_STRING,PL_INT}},
 { F_SQUICK,    "RLD$"  , f_rlds ,1,1   ,{PL_STRING}},
 { F_SQUICK,    "RLE$"  , f_rles ,1,1   ,{PL_STRING}},

 { F_IQUICK,    "SPACE$"  , f_spaces ,1,1   ,{PL_INT}},
 { F_PLISTE,  "STR$"    , f_strs ,1,4   ,{PL_NUMBER,PL_INT,PL_INT,PL_INT}},
 { F_PLISTE,  "STRING$" , f_strings ,2,2   ,{PL_INT,PL_STRING}},
 { F_SQUICK,    "SYSTEM$"    , f_systems ,1,1   ,{PL_STRING}},
 { F_SQUICK,    "TERMINALNAME$"    , f_terminalnames ,1,1 ,{PL_FILENR}},
#ifdef TINE
 { F_SQUICK,    "TINEGET$"    , f_tinegets ,1,1   ,{PL_STRING}},
 { F_SQUICK,    "TINEINFO$"   , f_tineinfos ,1,1   ,{PL_STRING}},
 { F_PLISTE,    "TINEQUERY$"  , f_tinequerys ,2,2   ,{PL_STRING,PL_INT}},
 { F_SQUICK,    "TINEUNIT$"   , f_tineunits ,1,1   ,{PL_STRING}},
#endif
 { F_SQUICK,    "TRIM$"   , f_trims ,1,1   ,{PL_STRING}},

 { F_SQUICK,    "UCASE$"    , f_uppers ,1,1   ,{PL_STRING}},
 { F_SQUICK,    "UNCOMPRESS$" , f_uncompresss ,1,1   ,{PL_STRING}},
 { F_IQUICK,    "UNIXDATE$" , f_unixdates ,1,1   ,{PL_INT}},
 { F_IQUICK,    "UNIXTIME$" , f_unixtimes ,1,1   ,{PL_INT}},
 { F_SQUICK,    "UPPER$"    , f_uppers ,1,1   ,{PL_STRING}},
 { F_SQUICK,    "XTRIM$"   , f_xtrims ,1,1   ,{PL_STRING}},

};
const int anzpsfuncs=sizeof(psfuncs)/sizeof(SFUNCTION);

int f_fak(int k) {
  int i,s=1;
  for(i=2;i<=k;i++) {s=s*i;}
  return(s);
}

int vergleich(char *w1,char *w2) {
  int v;
  int e=type2(w1);
  if((e | INTTYP | FLOATTYP)!=(type2(w2) | INTTYP | FLOATTYP )) {
    puts("Typen ungleich bei Vergleich!");
    printf("1: %d    2: %d \n",type2(w1),type2(w2));
    return(-1);
  }
  if(e & ARRAYTYP) {
    puts("Arrays an dieser Stelle noch nicht m�glich.");
    return(0);
  }
  else if(e & STRINGTYP) {
    STRING a,b;
    a=string_parser(w1);
    b=string_parser(w2);
    v=(a.len-b.len);
    if (v==0) v=memcmp(a.pointer,b.pointer,a.len);
    free(a.pointer);free(b.pointer);
  } else {
    double x=(parser(w1)-parser(w2));
    if(x==0) return(0);
    else if(x<0) return(-1);
    else return(1);
  }
  return(v);
}
int f_instr(PARAMETER *plist,int e) {
  int start=1;
  char *pos=NULL;
  if(e>=2) {
    if(e==3) start=min(plist[0].integer,max(1,plist[2].integer));
    pos=(char *)memmem(&(((char *)(plist[0].pointer))[start-1]),plist[0].integer-start+1,plist[1].pointer,plist[1].integer);
    if(pos!=NULL) return((int)(pos-(char *)plist[0].pointer)+1);
  } return(0);
}
int f_rinstr(PARAMETER *plist,int e) {
  char *pos=NULL,*n;
  int start;
  if(e>=2) {
    start=plist[0].integer;
    if(e==3) start=min(plist[0].integer,max(1,plist[2].integer));
    pos=rmemmem(plist[0].pointer,start-1,plist[1].pointer,plist[1].integer);
    if(pos!=NULL) return((int)(pos-(char *)plist[0].pointer)+1);
  } return(0);
}
#ifndef WINDOWS
  #include <fnmatch.h>
#else
  #include "Windows.extension/fnmatch.h"
#endif
int f_glob(PARAMETER *plist,int e) {
  char *pos=NULL,*n;
  int flags=FNM_NOESCAPE;
  if(e>=2) {
    if(e==3) flags^=plist[2].integer;
    flags=fnmatch(plist[1].pointer,plist[0].pointer,flags);
    if(flags==0) return(-1);
  } return(0);
}
#ifndef NOGRAPHICS
int f_form_alert(PARAMETER *plist,int e) {
  if(e==2) return(form_alert(plist[0].integer,plist[1].pointer));
  else return(-1);
}
int f_form_center(PARAMETER *plist,int e) {
  int x,y,w,h;
  graphics();
  gem_init();
  if(e==1) return(form_center(plist[0].integer,&x,&y,&w,&h));
  else if(e==5) {
    e=form_center(plist[0].integer,&x,&y,&w,&h);
    if(plist[1].typ!=PL_LEER) varcastint(plist[1].integer,plist[1].pointer,x);
    if(plist[2].typ!=PL_LEER) varcastint(plist[2].integer,plist[2].pointer,y);
    if(plist[3].typ!=PL_LEER) varcastint(plist[3].integer,plist[3].pointer,w);
    if(plist[4].typ!=PL_LEER) varcastint(plist[4].integer,plist[4].pointer,h);
    return(e);
  } else return(-1);
}
int f_form_dial(PARAMETER *plist,int e) {
  if(e==9) {
    graphics();
    gem_init();
    return(form_dial(plist[0].integer,plist[1].integer,
  plist[2].integer,plist[3].integer,plist[4].integer,plist[5].integer,
  plist[6].integer,plist[7].integer,plist[8].integer));
  } else return(-1);
}
int f_form_do(PARAMETER *plist,int e) {
  if(e==1) {
    graphics();
    gem_init();
    return(form_do((char *)plist[0].integer));
  } else return(-1);
}
int f_objc_draw(PARAMETER *plist,int e) {
  if(e==5) {
    graphics();
    gem_init();
    return(objc_draw((char *)plist[0].integer,plist[1].integer
    ,plist[2].integer,plist[3].integer,plist[4].integer));
  } else return(-1);
}
int f_objc_find(PARAMETER *plist,int e) {
  if(e==3) {
    return(objc_find((char *)plist[0].integer,plist[1].integer
    ,plist[2].integer));
  } else return(-1);
}
int f_objc_offset(PARAMETER *plist,int e) {
  int x,y;
  if(e==4) {
    if(plist[2].integer&FLOATTYP) x=(int)*((double *)plist[2].pointer);
    else if(plist[2].integer&INTTYP) x=*((int *)plist[2].pointer);
    else xberror(58,""); /* Variable hat falschen Typ */
    if(plist[3].integer&FLOATTYP) y=(int)*((double *)plist[3].pointer);
    else if(plist[3].integer&INTTYP) y=*((int *)plist[3].pointer);
    else xberror(58,""); /* Variable hat falschen Typ */
    e=objc_offset((char *)plist[0].integer,plist[1].integer,&x,&y);
    if(plist[2].integer&FLOATTYP) *((double *)plist[2].pointer)=(double)x;
    else if(plist[2].integer&INTTYP) *((int *)plist[2].pointer)=x;
     if(plist[3].integer&FLOATTYP) *((double *)plist[3].pointer)=(double)y;
    else if(plist[3].integer&INTTYP) *((int *)plist[3].pointer)=y;
    return(e);
  } else return(-1);
}
int f_get_color(PARAMETER *plist,int e) {
  if(e==3) {
    graphics();
    return(get_color(plist[0].integer,plist[1].integer,plist[2].integer));
  } else return(-1);
}
int f_rsrc_gaddr(PARAMETER *plist,int e) {
  int i;
  char *ptr;
  if(e==2) {
    i=rsrc_gaddr(plist[0].integer,plist[1].integer,&ptr);
    if(i>0) return((int)ptr);
  } return(-1);
}
#endif
double parser(char *funktion){  /* Rekursiver num. Parser */
  char *pos,*pos2;
  char s[strlen(funktion)+1],w1[strlen(funktion)+1],w2[strlen(funktion)+1];
  int e,vnr;

  /* printf("Parser: <%s>\n");*/

  /* Logische Operatoren AND OR NOT ... */

  if(searchchr2_multi(funktion,"&|")!=NULL) {
    if(wort_sepr2(s,"&&",TRUE,w1,w2)>1)     return((double)((int)parser(w1) & (int)parser(w2)));
    if(wort_sepr2(s,"||",TRUE,w1,w2)>1)     return((double)((int)parser(w1) | (int)parser(w2)));
  }
  xtrim(funktion,TRUE,s);  /* Leerzeichen vorne und hinten entfernen, Grossbuchstaben */

if(searchchr2(funktion,' ')!=NULL) {
  if(wort_sepr2(s," AND ",TRUE,w1,w2)>1)  return((double)((int)parser(w1) & (int)parser(w2)));    /* von rechts !!  */
  if(wort_sepr2(s," OR ",TRUE,w1,w2)>1)   return((double)((int)parser(w1) | (int)parser(w2)));
  if(wort_sepr2(s," NAND ",TRUE,w1,w2)>1) return((double)~((int)parser(w1) & (int)parser(w2)));
  if(wort_sepr2(s," NOR ",TRUE,w1,w2)>1)  return((double)~((int)parser(w1) | (int)parser(w2)));
  if(wort_sepr2(s," XOR ",TRUE,w1,w2)>1)  return((double)((int)parser(w1) ^ (int)parser(w2)));	
  if(wort_sepr2(s," EOR ",TRUE,w1,w2)>1)  return((double)((int)parser(w1) ^ (int)parser(w2)));	
  if(wort_sepr2(s," EQV ",TRUE,w1,w2)>1)  return((double)~((int)parser(w1) ^ (int)parser(w2)));
  if(wort_sepr2(s," IMP ",TRUE,w1,w2)>1)  return((double)(~((int)parser(w1) ^ (int)parser(w2)) | (int)parser(w2)));
  if(wort_sepr2(s," MOD ",TRUE,w1,w2)>1)  return(fmod(parser(w1),parser(w2)));
  if(wort_sepr2(s," DIV ",TRUE,w1,w2)>1) {
    int nenner=(int)parser(w2);
    if(nenner) return((double)((int)parser(w1) / nenner));
    else {
      xberror(0,w2); /* Division durch 0 */
      return(0);
    }
  }
  if(wort_sepr2(s,"NOT ",TRUE,w1,w2)>1) {
    if(strlen(w1)==0) return((double)(~(int)parser(w2)));    /* von rechts !!  */
    /* Ansonsten ist NOT Teil eines Variablennamens */
  }
}

  /* Erst Vergleichsoperatoren mit Wahrheitwert abfangen (lowlevel < Addition)  */
if(searchchr2_multi(s,"<=>")!=NULL) {
  if(wort_sep2(s,"==",TRUE,w1,w2)>1)      return(vergleich(w1,w2)?0:-1);
  if(wort_sep2(s,"<>",TRUE,w1,w2)>1) return(vergleich(w1,w2)?-1:0);
  if(wort_sep2(s,"><",TRUE,w1,w2)>1) return(vergleich(w1,w2)?-1:0);
  if(wort_sep2(s,"<=",TRUE,w1,w2)>1) return((vergleich(w1,w2)<=0)?-1:0);
  if(wort_sep2(s,">=",TRUE,w1,w2)>1) return((vergleich(w1,w2)>=0)?-1:0);
  if(wort_sep(s,'=',TRUE,w1,w2)>1)   return(vergleich(w1,w2)?0:-1);
  if(wort_sep(s,'<',TRUE,w1,w2)>1)   return((vergleich(w1,w2)<0)?-1:0);
  if(wort_sep(s,'>',TRUE,w1,w2)>1)   return((vergleich(w1,w2)>0)?-1:0);
}
 /* Addition/Subtraktion/Vorzeichen  */
if(searchchr2_multi(s,"+-")!=NULL) {
  if(wort_sep_e(s,'+',TRUE,w1,w2)>1) {
    if(strlen(w1)) return(parser(w1)+parser(w2));
    else return(parser(w2));   /* war Vorzeichen + */
  }
  if(wort_sepr_e(s,'-',TRUE,w1,w2)>1) {       /* von rechts !!  */
    if(strlen(w1)) return(parser(w1)-parser(w2));
    else return(-parser(w2));   /* war Vorzeichen - */
  }
}
if(searchchr2_multi(s,"*/^")!=NULL) {
  if(wort_sepr(s,'*',TRUE,w1,w2)>1) {
    if(strlen(w1)) return(parser(w1)*parser(w2));
    else {
      printf("Pointer noch nicht integriert! %s\n",w2);   /* war pointer - */
      return(0);
    }
  }
  if(wort_sepr(s,'/',TRUE,w1,w2)>1) {
    if(strlen(w1)) {
      double nenner;
      nenner=parser(w2);
      if(nenner!=0.0) return(parser(w1)/nenner);    /* von rechts !!  */
      else { xberror(0,w2); return(0);  } /* Division durch 0 */
    } else { xberror(51,w2); return(0); }/* "Parser: Syntax error?! "  */
  }
  if(wort_sepr(s,'^',TRUE,w1,w2)>1) {
    if(strlen(w1)) return(pow(parser(w1),parser(w2)));    /* von rechts !!  */
    else { xberror(51,w2); return(0); } /* "Parser: Syntax error?! "  */
  }
}
  if(*s=='(' && s[strlen(s)-1]==')')  { /* Ueberfluessige Klammern entfernen */
    s[strlen(s)-1]=0;
    return(parser(s+1));
    /* SystemFunktionen Subroutinen und Arrays */
  } else {
    pos=searchchr(s, '(');
    if(pos!=NULL) {
      pos2=s+strlen(s)-1;
      *pos++=0;

      if(*pos2!=')') xberror(51,w2); /* "Parser: Syntax error?! "  */
      else {                         /* $-Funktionen und $-Felder   */
        *pos2=0;

        /* Benutzerdef. Funktionen */
        if(*s=='@') return(do_funktion(s+1,pos));
	else {
	  /* Liste durchgehen */
	  int i=0,a=anzpfuncs-1,b,l=strlen(s);
          for(b=0; b<l; b++) {
            while(s[b]>(pfuncs[i].name)[b] && i<a) i++;
            while(s[b]<(pfuncs[a].name)[b] && a>i) a--;
            if(i==a) break;
          }
          if(strcmp(s,pfuncs[i].name)==0) {
	     /* printf("Funktion %s gefunden. Nr. %d\n",pfuncs[i].name,i); */
	      if((pfuncs[i].opcode&FM_TYP)==F_SIMPLE || pfuncs[i].pmax==0) {
	        if(pfuncs[i].opcode&F_IRET)
		  return((double)((int (*)())pfuncs[i].routine)());
		else return((pfuncs[i].routine)());
	      } else if((pfuncs[i].opcode&FM_TYP)==F_ARGUMENT) {
	      	if(pfuncs[i].opcode&F_IRET) return((double)((int (*)())pfuncs[i].routine)(pos));
		else return((pfuncs[i].routine)(pos));
	      } else if((pfuncs[i].opcode&FM_TYP)==F_PLISTE) {
		 PARAMETER *plist;
                 int e=make_pliste(pfuncs[i].pmin,pfuncs[i].pmax,(short *)pfuncs[i].pliste,pos,&plist);
                 double a;
                 if(pfuncs[i].opcode&F_IRET) a=(double)((int (*)())pfuncs[i].routine)(plist,e);
	         else a=(pfuncs[i].routine)(plist,e);
	         if(e!=-1) free_pliste(e,plist);
	         return(a);
	      } else if(pfuncs[i].pmax==1 && (pfuncs[i].opcode&FM_TYP)==F_DQUICK) {
	      	if(pfuncs[i].opcode&F_IRET)
		  return((double)((int (*)())pfuncs[i].routine)(parser(pos)));
		else return((pfuncs[i].routine)(parser(pos)));
	      } else if(pfuncs[i].pmax==1 && (pfuncs[i].opcode&FM_TYP)==F_IQUICK) {
	      	if(pfuncs[i].opcode&F_IRET) return((double)((int (*)())pfuncs[i].routine)((int)parser(pos)));
		else return((pfuncs[i].routine)((int)parser(pos)));
	      } else if(pfuncs[i].pmax==2 && (pfuncs[i].opcode&FM_TYP)==F_DQUICK) {
	       	 char w1[strlen(pos)+1],w2[strlen(pos)+1];
	         int e;
		 double val1,val2;
	         if((e=wort_sep(pos,',',TRUE,w1,w2))==1) {
		   xberror(56,""); /* Falsche Anzahl Parameter */
		   val1=parser(w1); val2=0;
	         } else if(e==2) {
	           val1=parser(w1); val2=parser(w2);
	         }
                if(pfuncs[i].opcode&F_IRET) return((double)((int (*)())pfuncs[i].routine)(val1,val2));
		else return((pfuncs[i].routine)(val1,val2));
	      } else if(pfuncs[i].pmax==2 && (pfuncs[i].opcode&FM_TYP)==F_IQUICK) {
	       	 char w1[strlen(pos)+1],w2[strlen(pos)+1];
	         int e;
		 int val1,val2;
	         if((e=wort_sep(pos,',',TRUE,w1,w2))==1) {
		   xberror(56,""); /* Falsche Anzahl Parameter */
		   val1=(int)parser(w1); val2=0;
	         } else if(e==2) {
	           val1=(int)parser(w1); val2=(int)parser(w2);
	         }
                if(pfuncs[i].opcode&F_IRET) return((double)((int (*)())pfuncs[i].routine)(val1,val2));
		else return((pfuncs[i].routine)(val1,val2));
	
	      } else if(pfuncs[i].pmax==1 && (pfuncs[i].opcode&FM_TYP)==F_SQUICK) {
                STRING test=string_parser(pos);
		double erg;
		test.pointer=realloc(test.pointer,test.len+1);
		test.pointer[test.len]=0;
	        if(pfuncs[i].opcode&F_IRET) erg=(double)((int (*)())pfuncs[i].routine)(test);
		else erg=(pfuncs[i].routine)(test);
		free(test.pointer);
		return(erg);
	      } else printf("Interner ERROR. Funktion nicht korrekt definiert. %s\n",s);
	   /* Nicht in der Liste ? Dann kann es noch ARRAY sein   */
	
          } else if(type2(s) & FLOATTYP) {
            if((vnr=variable_exist(s,FLOATARRAYTYP))!=-1) return(floatarrayinhalt(vnr,pos));
	    else { xberror(15,s); return(0); } /* Feld nicht dimensioniert  */
          } else if(type2(s) & INTTYP) {
	    char *r=varrumpf(s);
	    if((vnr=variable_exist(r,INTARRAYTYP))!=-1) return((double)intarrayinhalt(vnr,pos));
	    else { xberror(15,s); return(0); }  /* Feld nicht dimensioniert  */
	    free(r);
	  } else { xberror(15,s); return(0); }  /* Feld nicht dimensioniert  */
        }
      }
    } else {  /* Also keine Klammern */
      /* Dann Systemvariablen und einfache Variablen */

	  /* Liste durchgehen */
	  char c=*s;
	  int i=0,a=anzsysvars-1,b,l=strlen(s);
          for(b=0; b<l; c=s[++b]) {
            while(c>(sysvars[i].name)[b] && i<a) i++;
            while(c<(sysvars[a].name)[b] && a>i) a--;
            if(i==a) break;
          }
          if(strcmp(s,sysvars[i].name)==0) {
	    /*  printf("Sysvar %s gefunden. Nr. %d\n",sysvars[i].name,i);*/
	   if((sysvars[i].opcode)==PL_INT) return((double)((int (*)())sysvars[i].routine)());
	   if((sysvars[i].opcode)==PL_FLOAT) return((sysvars[i].routine)());
          }
      /* erst integer abfangen (xxx% oder xxx&), dann rest */

      if(*s=='@')                              return(do_funktion(s+1,""));
      if((vnr=variable_exist(s,FLOATTYP))!=-1) return(variablen[vnr].zahl);
      if(s[strlen(s)-1]=='%') {
        s[strlen(s)-1]=0;
        if((vnr=variable_exist(s,INTTYP))!=-1) return((double)variablen[vnr].opcode);
        return(0);
      } else return(atof(s));  /* Jetzt nur noch Zahlen (hex, oct etc ...)*/
    }
  }
  xberror(51,s); /* Syntax error */
  return(0);
}

ARRAY f_smula(PARAMETER *plist, int e) {
	   ARRAY ergeb;
	   ergeb.typ=plist[0].typ;
	   ergeb.dimension=plist[0].integer;
	   ergeb.pointer=plist[0].pointer;
	   ergeb=double_array(ergeb);
	   array_smul(ergeb,plist[1].real);
	   return(ergeb);
}
ARRAY f_nullmat(PARAMETER *plist, int e) {
    int dimlist[2]={plist[0].integer,plist[1].integer};
    return(nullmatrix(FLOATARRAYTYP,e,dimlist));
}
ARRAY f_einsmat(PARAMETER *plist, int e) {
    int dimlist[2]={plist[0].integer,plist[0].integer};
    return(einheitsmatrix(FLOATARRAYTYP,2,dimlist));
}

extern double *SVD(double *a, double *w, double *v,int anzzeilen, int anzspalten);
extern double *backsub(double *, double *, double *, double *,int,int);

double *makeSVD(double *v1,double *m1,int anzzeilen, int anzspalten) {
  int i,j;
  double maxsing=0;
  int elim=0;
  int fsing=0;
  double *ergebnis;

  double *u = malloc(sizeof(double)*anzzeilen*anzspalten);
  double *v = malloc(sizeof(double)*anzspalten*anzspalten);
  double *singulars = malloc(sizeof(double)*anzspalten);

  memcpy(u,m1,sizeof(double)*anzzeilen*anzspalten);
  singulars=SVD(u,singulars,v,anzzeilen,anzspalten);

#ifdef DEBUG
  printf("Eigenwerte:\n");
  for(i=0;i<anzspalten;i++) printf("%g\n",singulars[i]);
#endif
/* Groessten Singulaerwert rausfinden */

  for(i=0;i<anzspalten;i++) {
    if(fabs(singulars[i])>maxsing) maxsing=fabs(singulars[i]);
  }

/* Zaehle Anzahl der Singulaeren Werte (d.h. Eigenwerte=0) */
/* Akzeptiere nur Eigenwerte die mindestens 1e-10 vom groessten sind,
   ansonsten setze sie zu 0 */

  for(i=0;i<anzspalten;i++) {
    if(singulars[i]==0) fsing++;
    if(fabs(singulars[i])/maxsing<1e-10 && singulars[i]) {
      printf("** %g\n",singulars[i]);
      singulars[i]=0;
      elim++;
    }
  }
  if(fsing || elim) printf("Found %d singularities and eliminated another %d.\n",fsing,elim);
  ergebnis=backsub(singulars,u,v,v1,anzzeilen,anzspalten);
  free(u);free(v);
  free(singulars);
  return(ergebnis);
}

makeSVD2(double *v1,double *m1,int anzzeilen, int anzspalten, double *ergeb) {
  double *x;
  x=makeSVD(v1,m1,anzzeilen,anzspalten);
  memcpy(ergeb,x,sizeof(double)*anzspalten);
}

/* Gleichungssystem loesen  d=Mx    x()=SOLVE(m(),d())*/
ARRAY f_solvea(PARAMETER *plist, int e) {
  ARRAY ergeb;
  int anzzeilen,anzspalten;
  ergeb.typ=plist[0].typ;
  ergeb.dimension=1;
  if(plist[0].integer!=2) xberror(81,""); /* "Matrizen haben nicht die gleiche Ordnung" */
  if(plist[1].integer!=1) xberror(81,""); /* "Matrizen haben nicht die gleiche Ordnung" */
  anzspalten=*((int *)(plist[0].pointer+sizeof(int)));
  anzzeilen=*((int *)(plist[0].pointer));

  if(anzzeilen!=*((int *)(plist[1].pointer))) xberror(81,""); /* "Matrizen haben nicht die gleiche Ordnung" */

  ergeb.pointer=malloc(INTSIZE+anzspalten*sizeof(double));
  *((int *)ergeb.pointer)=anzspalten;
  makeSVD2((double *)(plist[1].pointer+plist[1].integer*INTSIZE),(double *)(plist[0].pointer+plist[0].integer*INTSIZE),anzzeilen,anzspalten, (double *)(ergeb.pointer+INTSIZE));
  return(ergeb);
}

#ifdef CONTROL
ARRAY f_csvgeta(char *pos) {
  int o=0,nn=0;
  if(e>1) nn=plist[1].integer;
  if(e>2) o=plist[2].integer;
  return(csvget(plist[0].pointer,nn,o));
}
#endif
#ifdef TINE
ARRAY f_tinegeta(PARAMETER *plist, int e) {
  int o=0,nn=0;
  if(e>1) nn=plist[1].integer;
  if(e>2) o=plist[2].integer;
  return(tinevget(plist[0].pointer,nn,o));
}
ARRAY f_tinehistorya(PARAMETER *plist, int e) {
	  return(tinehistory(plist[0].pointer,plist[1].integer,plist[2].integer));
}
#endif
#ifdef DOOCS
ARRAY doocsnames(char *n);
ARRAY f_doocsgeta(PARAMETER *plist, int e) {
  int o=0,nn=0;
  if(e>1) nn=plist[1].integer;
  if(e>2) o=plist[2].integer;
  return(doocsvget(plist[0].pointer,nn,o));
}
ARRAY f_doocsnames(PARAMETER *plist, int e) {
  return(doocsnames(plist[0].pointer));
}
#endif

const AFUNCTION pafuncs[]= {  /* alphabetisch !!! */
 { F_ARGUMENT,  "!nulldummy",  f_nop ,0,0   ,{0}},
 { F_PLISTE,    "0"         , f_nullmat ,2,2   ,{PL_INT,PL_INT}},
 { F_PLISTE,    "1"         , f_einsmat ,1,1   ,{PL_INT}},
#ifdef CONTROL
 { F_PLISTE,    "CSGET"     , f_csvgeta ,1,3   ,{PL_STRING,PL_INT,PL_INT}},
 { F_PLISTE,    "CSVGET"    , f_csvgeta ,1,3   ,{PL_STRING,PL_INT,PL_INT}},
#endif
 { F_SQUICK,    "CVA"       , string_to_array ,1,1   ,{PL_STRING}},
#ifdef DOOCS
 { F_PLISTE,    "DOOCSGET"     , f_doocsgeta ,1,3   ,{PL_STRING,PL_INT,PL_INT}},
 { F_PLISTE,    "DOOCSNAMES"     , f_doocsnames ,1,1   ,{PL_STRING}},
 { F_PLISTE,    "DOOCSVGET"    , f_doocsgeta ,1,3   ,{PL_STRING,PL_INT,PL_INT}},
#endif

 { F_AQUICK,  "INV"         , inv_array ,1,1   ,{PL_NARRAY}},
 { F_PLISTE,  "SMUL"        , f_smula ,2,2   ,{PL_ARRAY,PL_FLOAT}},
 { F_PLISTE,  "SOLVE"       , f_solvea ,2,2   ,{PL_ARRAY,PL_ARRAY}},

#ifdef TINE
 { F_PLISTE,    "TINEGET"     , f_tinegeta ,1,3   ,{PL_STRING,PL_INT,PL_INT}},
 { F_PLISTE,    "TINEVGET"    , f_tinegeta ,1,3   ,{PL_STRING,PL_INT,PL_INT}},
 { F_PLISTE,    "TINEHISTORY" , f_tinehistorya ,3,3   ,{PL_STRING,PL_INT,PL_INT}},
#endif
 { F_AQUICK,    "TRANS"     , trans_array ,1,1   ,{PL_ARRAY}},

};
const int anzpafuncs=sizeof(pafuncs)/sizeof(AFUNCTION);

ARRAY array_parser(char *funktion) { /* Array-Parser  */
/* Rekursiv und so, dass dynamische Speicherverwaltung ! */
/* Trenne ersten Token ab, und uebergebe rest derselben Routine */

/* Zum Format:

   a()    ganzes Array
   a(1,2,:,3,:) nur Teil des Arrays. Die angegebenen Indizies werden
                festgehalten. Es resultiert ein 2-dimensionales Array
		*/


  char s[strlen(funktion)+1],w1[strlen(funktion)+1],w2[strlen(funktion)+1];
  char *pos;
  int e;
  strcpy(s,funktion);
  xtrim(s,TRUE,s);  /* Leerzeichen vorne und hinten entfernen */
//  printf("ARRAY_PARSER: \n");
  if(wort_sep(s,'+',TRUE,w1,w2)>1) {
    if(strlen(w1)) {
      ARRAY zw1=array_parser(w1);
      ARRAY zw2=array_parser(w2);
      array_add(zw1,zw2);
      free_array(zw2);
      return(zw1);
    } else return(array_parser(w2));
  } else if(wort_sepr(s,'-',TRUE,w1,w2)>1) {
    if(strlen(w1)) {
      ARRAY zw1=array_parser(w1);
      ARRAY zw2=array_parser(w2);
      array_sub(zw1,zw2);
      free_array(zw2);
      return(zw1);
    } else {
      ARRAY zw2=array_parser(w2);
      array_smul(zw2,-1);
      return(zw2);
    }
  } else if(wort_sepr(s,'*',TRUE,w1,w2)>1) {
    if(strlen(w1)) {
      if(type2(w1) & ARRAYTYP) {
        ARRAY zw1=array_parser(w1);
	ARRAY zw2=array_parser(w2);
        ARRAY ergebnis=mul_array(zw1,zw2);
        free_array(zw1); free_array(zw2);
        return(ergebnis);
      } else {    /* Skalarmultiplikation */
        ARRAY zw2=array_parser(w2);
	array_smul(zw2,parser(w1));
	return(zw2);
      }
    } else xberror(51,""); /*Syntax Error*/
  } else if(wort_sepr(s,'^',TRUE,w1,w2)>1) {
    ARRAY zw2,zw1=array_parser(w1);
    e=(int)parser(w2);
    if(e<0) xberror(51,""); /*Syntax Error*/
    else if(e==0) {
      zw2=zw1;
      zw1=einheitsmatrix(zw2.typ,zw2.dimension,zw2.pointer);
      free_array(zw2);
    } else if(e>1) {
      int i;
      for(i=1;i<e;i++) {
        zw2=mul_array(zw1,zw1);
	free_array(zw1);
	zw1=zw2;
      }
    }
    return(zw1);
  } else if(s[0]=='(' && s[strlen(s)-1]==')')  { /* Ueberfluessige Klammern entfernen */
    s[strlen(s)-1]=0;
    return(array_parser(s+1));
  } else {/* SystemFunktionen, Konstanten etc... */
    if(*s=='[' && s[strlen(s)-1]==']') {  /* Konstante */
      s[strlen(s)-1]=0;
      return(array_const(s+1));
    }
    pos=searchchr(s,'(');
    if(pos!=NULL) {
      char *pos2=s+strlen(s)-1;
      *pos++=0;
      if(*pos2!=')') {
         xberror(51,w2); /* "Parser: Syntax error?! "  */
      } else {                         /* $-Funktionen und $-Felder   */
	  /* Liste durchgehen */
	  int i=0,a=anzpafuncs-1,b,l=strlen(s);
          *pos2=0;
          for(b=0; b<l; b++) {
            while(s[b]>(pafuncs[i].name)[b] && i<a) i++;
            while(s[b]<(pafuncs[a].name)[b] && a>i) a--;
            if(i==a) break;
          }
	  if(strcmp(s,pafuncs[i].name)==0) {
	    /*  printf("Funktion %s gefunden. Nr. %d\n",pafuncs[i].name,i); */
	      if((pafuncs[i].opcode&FM_TYP)==F_SIMPLE || pafuncs[i].pmax==0) {
		return((pafuncs[i].routine)());
	      } else if((pafuncs[i].opcode&FM_TYP)==F_ARGUMENT) {
	     	return((pafuncs[i].routine)(pos));
	      } else if((pafuncs[i].opcode&FM_TYP)==F_PLISTE) {
		 PARAMETER *plist;
                 int e=make_pliste(pafuncs[i].pmin,pafuncs[i].pmax,(short *)pafuncs[i].pliste,pos,&plist);
                 ARRAY a=(pafuncs[i].routine)(plist,e);
	         if(e!=-1) free_pliste(e,plist);
	         return(a);
	      } else if(pafuncs[i].pmax==1 && (pafuncs[i].opcode&FM_TYP)==F_AQUICK) {
	        ARRAY ergebnis,a=array_parser(pos);
		ergebnis=(pafuncs[i].routine)(a);
		free_array(a);
	      	return(ergebnis);
	      } else if(pafuncs[i].pmax==1 && (pafuncs[i].opcode&FM_TYP)==F_SQUICK) {
	        ARRAY ergebnis;
		STRING a=string_parser(pos);
		ergebnis=(pafuncs[i].routine)(a);
		free_string(a);
	      	return(ergebnis);
	      } else printf("Interner ERROR. Funktion nicht korrekt definiert. %s\n",s);
          }/* Nicht in der Liste ?    */
         if(strcmp(s,"INP%")==0) {
	   char w1[strlen(pos)+1],w2[strlen(pos)+1];
	   int i,nn;
	   ARRAY ergeb;
           FILE *fff=stdin;
	   wort_sep(pos,',',TRUE,w1,w2);
	   i=get_number(w1);
	   nn=(int)parser(w2);
	
           ergeb.typ=INTTYP;
	   ergeb.dimension=1;
	   ergeb.pointer=malloc(INTSIZE+nn*sizeof(int));
	   ((int *)(ergeb.pointer))[0]=nn;
           if(filenr[i]) {
	     int *varptr=ergeb.pointer+INTSIZE;
             fff=dptr[i];
             fread(varptr,sizeof(int),nn,fff);
             return(ergeb);
           } else xberror(24,""); /* File nicht geoeffnet */
           return(ergeb);
	 } else {
	    int vnr;
	    char *r=varrumpf(s);
	    ARRAY ergebnis;
	   /* Hier sollten keine Funktionen mehr auftauchen  */
	   /* Jetzt uebergebe spezifiziertes Array, evtl. reduziert*/
	   if(strlen(pos)==0) {
	     if((vnr=variable_exist(r,FLOATARRAYTYP))!=-1)  ergebnis=copy_var_array(vnr);
	     else if((vnr=variable_exist(r,INTARRAYTYP))!=-1)    ergebnis=copy_var_array(vnr);
	     else if((vnr=variable_exist(r,STRINGARRAYTYP))!=-1) ergebnis=copy_var_array(vnr);
             else xberror(15,s);  /* Feld nicht dimensioniert  */
	   } else {
	     if((vnr=variable_exist(r,FLOATARRAYTYP))!=-1) {
	       char w1[strlen(pos)+1],w2[strlen(pos)+1];
	       int i,e,rdim=0,ndim=0,anz=1,anz2=1,j,k;
	       int indexe[variablen[vnr].opcode];
	       int indexo[variablen[vnr].opcode];
	       int indexa[variablen[vnr].opcode];
	
	     /* Dimension des reduzierten Arrays bestimmen */
	       ergebnis.typ=FLOATTYP;
	       e=wort_sep(pos,',',TRUE,w1,w2);
	       while(e) {
	         if(w1[0]!=':' && w1[0]!=0) {
		   indexo[ndim++]=(int)parser(w1);
		   rdim++;
		 } else indexo[ndim++]=-1;
		
	         e=wort_sep(w2,',',TRUE,w1,w2);
	       }
	
             /* Dimensionierung uebertragen */
	       ergebnis.dimension=max(variablen[vnr].opcode-rdim,1);
	       ergebnis.pointer=malloc(INTSIZE*ergebnis.dimension);
	       rdim=0;ndim=0;anz=1;
	       e=wort_sep(pos,',',TRUE,w1,w2);
	       while(e) {
	         indexa[rdim]=anz;		
	         if(w1[0]==':' || w1[0]==0) {
		
		   ((int *)(ergebnis.pointer))[ndim++]=((int *)(variablen[vnr].pointer))[rdim];
		   anz=anz*(((int *)variablen[vnr].pointer)[rdim]);
		 }
		 rdim++;
	         e=wort_sep(w2,',',TRUE,w1,w2);
	       }	

 	       ergebnis.pointer=realloc(ergebnis.pointer,INTSIZE*ergebnis.dimension+anz*sizeof(double));

	      /*Loop fuer die Komprimierung */

	       for(j=0;j<anz;j++) {
	         int jj=j;
	         /* Indexliste aus anz machen */
                 for(k=variablen[vnr].opcode-1;k>=0;k--) {
		   if(indexo[k]==-1) {
		     indexe[k]=jj/indexa[k];
		     jj=jj % indexa[k];
		
		   } else indexe[k]=indexo[k];
		 }
		 if(jj!=0) puts("Rechnung geht nicht auf.");
		 /* Testen ob passt  */
	         /*printf("j=%d : indexe[]=",j);*/
		 anz2=0;
	         for(k=0;k<variablen[vnr].opcode;k++) {
		   /*printf(" %d",indexe[k]);*/
		   anz2=anz2*((int *)variablen[vnr].pointer)[k]+indexe[k];
		 }
	         /*printf("\n");
		 printf("--anz2=%d\n",anz2);*/

		 /* jetzt kopieren */
		
		 ((double *)(ergebnis.pointer+INTSIZE*ergebnis.dimension))[j]=((double *)(variablen[vnr].pointer+INTSIZE*variablen[vnr].opcode))[anz2];
	       }
	     } else if((vnr=variable_exist(r,INTARRAYTYP))!=-1) {
	       puts("Noch nicht m�glich...");
	     }  else if((vnr=variable_exist(s,STRINGARRAYTYP))!=-1) {
	       puts("Noch nicht m�glich...");
	     } else {
	       xberror(15,s);  /* Feld nicht dimensioniert  */
	       e=1;
  	       ergebnis=einheitsmatrix(FLOATARRAYTYP,1,&e);
	     }
	   }
	   free(r);
	   return(ergebnis);
        }
      }
    }
  }
  /* Offenbar war der String leer oder so */
  e=0;
  return(nullmatrix(FLOATARRAYTYP,0,&e));
}

STRING string_parser(char *);

char *s_parser(char *funktion) { /* String-Parser  */
  STRING e=string_parser(funktion);
  e.pointer=realloc(e.pointer,e.len+1);
  (e.pointer)[e.len]=0;
  return(e.pointer);
}
STRING string_parser(char *funktion) {
/* Rekursiv und so, dass dynamische Speicherverwaltung ! */
/* Trenne ersten Token ab, und uebergebe rest derselben Routine */

  STRING ergebnis;
  char v[strlen(funktion)+1],w[strlen(funktion)+1];

 // printf("S-Parser: <%s>\n",funktion);
  if(wort_sep(funktion,'+',TRUE,v,w)>1) {
    STRING t=string_parser(v);
    STRING u=string_parser(w);
    ergebnis.pointer=malloc(t.len+u.len+1);
    memcpy(ergebnis.pointer,t.pointer,t.len);
    memcpy(ergebnis.pointer+t.len,u.pointer,u.len+1);
    ergebnis.len=u.len+t.len;
    free(t.pointer);free(u.pointer);
    return(ergebnis);
  } else {
    char *pos,*pos2,*inhalt;
    int vnr;

  //  printf("s-parser: <%s>\n",funktion);
    strcpy(v,funktion);
    pos=searchchr(v, '(');
    if(pos!=NULL) {
      pos2=v+strlen(v)-1;
      *pos++=0;

      if(*pos2!=')') {
        xberror(51,v); /* "Parser: Syntax error?! "  */
        ergebnis=vs_error();
      } else {                         /* $-Funktionen und $-Felder   */
        *pos2=0;
       	
	if(*v=='@')     /* Funktion oder Array   */
 	  ergebnis=do_sfunktion(v+1,pos);	
        else {  /* Liste durchgehen */
	  int i=0,a=anzpsfuncs-1,b;    

          for(b=0; b<strlen(v); b++) {
            while(v[b]>(psfuncs[i].name)[b] && i<a) i++;
            while(v[b]<(psfuncs[a].name)[b] && a>i) a--;
            if(i==a) break;
          }
//          printf("s-parser: <%s>\n",funktion);
  
          if(strcmp(v,psfuncs[i].name)==0) {
//          printf("Funktion gefunden <%s>\n",psfuncs[i].name);
//          printf("opcode=%d pmax=%d  AQUICK=%d\n",psfuncs[i].opcode&FM_TYP,psfuncs[i].pmax,F_AQUICK);
	  
	      if((psfuncs[i].opcode&FM_TYP)==F_SIMPLE || psfuncs[i].pmax==0)
	        ergebnis=(psfuncs[i].routine)();
	      else if((psfuncs[i].opcode&FM_TYP)==F_ARGUMENT)
	      	ergebnis=(psfuncs[i].routine)(pos);
	      else if((psfuncs[i].opcode&FM_TYP)==F_PLISTE) {
	        PARAMETER *plist;
                 int e=make_pliste(psfuncs[i].pmin,psfuncs[i].pmax,(short *)psfuncs[i].pliste,pos,&plist);
                 ergebnis=(psfuncs[i].routine)(plist,e);
	         if(e!=-1) free_pliste(e,plist);
	      } else if(psfuncs[i].pmax==1 && (psfuncs[i].opcode&FM_TYP)==F_DQUICK) {
		ergebnis=(psfuncs[i].routine)(parser(pos));
	      } else if(psfuncs[i].pmax==1 && (psfuncs[i].opcode&FM_TYP)==F_IQUICK) {
		ergebnis=(psfuncs[i].routine)((int)parser(pos));
	      } else if(psfuncs[i].pmax==2 && (psfuncs[i].opcode&FM_TYP)==F_DQUICK) {
	       	 char w1[strlen(pos)+1],w2[strlen(pos)+1];
	         int e;
		 double val1,val2;
	         if((e=wort_sep(pos,',',TRUE,w1,w2))==1) {
		   xberror(56,""); /* Falsche Anzahl Parameter */
		   val1=parser(w1); val2=0;
	         } else if(e==2)  val1=parser(w1); val2=parser(w2);
                 ergebnis=(psfuncs[i].routine)(val1,val2);
	      } else if(psfuncs[i].pmax==1 && (psfuncs[i].opcode&FM_TYP)==F_SQUICK) {
                STRING test=string_parser(pos);
		test.pointer=realloc(test.pointer,test.len+1);
		test.pointer[test.len]=0;
	        ergebnis=(psfuncs[i].routine)(test);
		free(test.pointer);
	      } else if(psfuncs[i].pmax==1 && (psfuncs[i].opcode&FM_TYP)==F_AQUICK) {
                ARRAY test=array_parser(pos);
	        ergebnis=(psfuncs[i].routine)(test);
		free_array(test);
	      } else printf("Interner ERROR. Funktion nicht korrekt definiert. %s\n",v);
	    } else {/* Nicht in der Liste ? Dann kann es noch ARRAY sein   */
	     int vnr;
	     v[strlen(v)-1]=0;
	
             if((vnr=variable_exist(v,STRINGARRAYTYP))==-1) {
	       xberror(15,v);         /*Feld nicht definiert*/
	       ergebnis=create_string(NULL);
             } else {
	       int dim=variablen[vnr].opcode;
	       int indexliste[dim];
	       
	       if(make_indexliste(dim,pos,indexliste)==0)
	         ergebnis=varstringarrayinhalt(vnr,indexliste);
	       else ergebnis=create_string(NULL);
	    }
	  }
        }
      }
    } else {
      pos2=v+strlen(v)-1;
      if(*v=='"' && *pos2=='"') {  /* Konstante  */
        ergebnis.pointer=malloc(strlen(v)-2+1);
        ergebnis.len=strlen(v)-2;
        *pos2=0;
        memcpy(ergebnis.pointer,v+1,strlen(v)-2+1);
      } else if(*pos2!='$') {
        xberror(51,v); /* "Parser: Syntax error?! "  */
        ergebnis=vs_error();
      } else {                      /* einfache Variablen und Systemvariablen */
	/* Liste durchgehen */
	int i=0,a=anzsyssvars,b;
        for(b=0; b<strlen(v); b++) {
          while(v[b]>(syssvars[i].name)[b] && i<a) i++;
          while(v[b]<(syssvars[a].name)[b] && a>i) a--;
          if(i==a) break;
        }
        if(strcmp(v,syssvars[i].name)==0) {
	    /*  printf("Sysvar %s gefunden. Nr. %d\n",syssvars[i].name,i);*/
	  return((syssvars[i].routine)());
        }
        *pos2=0;
        if((vnr=variable_exist(v,STRINGTYP))==-1) {
          ergebnis=create_string(NULL);
        } else {
          ergebnis.pointer=malloc(variablen[vnr].opcode+1);
	  ergebnis.len=variablen[vnr].opcode;
	  memcpy(ergebnis.pointer,variablen[vnr].pointer,ergebnis.len);
        }
      }
    }
  }
  return(ergebnis);
}

          /* STR$(a[,b[,c[,d]]])     */


STRING hexoct_to_string(char n,PARAMETER *plist, int e) {
  STRING ergebnis;
  char formatter[20];
  int b=-1,c=13,mode=0;
  unsigned int a=plist[0].integer;
  if(e>1) b=min(50,max(0,plist[1].integer));
  if(e>2) c=min(50,max(0,plist[2].integer));
  if(e>3) mode=plist[3].integer;
	
  if(mode==0 && b!=-1) sprintf(formatter,"%%%d.%d%c",b,c,n);
  else if (mode==1 && b!=-1) sprintf(formatter,"%%0%d.%d%c",b,c,n);
  else  sprintf(formatter,"%%.13%c",n);
  ergebnis.pointer=malloc(31);
  sprintf(ergebnis.pointer,formatter,a);
  ergebnis.len=strlen(ergebnis.pointer);
  return(ergebnis);
}


double do_funktion(char *name,char *argumente) {
 char *buffer,*pos,*pos2,*pos3;
    int pc2;

    buffer=malloc(strlen(name)+1);
    strcpy(buffer,name);
    pos=argumente;

    pc2=procnr(buffer,2);
    if(pc2==-1)   xberror(44,buffer); /* Funktion  nicht definiert */
    else {
	if(do_parameterliste(pos,procs[pc2].parameterliste)) xberror(42,buffer); /* Zu wenig Parameter */
	else {
	  int oldbatch,osp=sp;
	  pc2=procs[pc2].zeile;
	  if(sp<STACKSIZE) {stack[sp++]=pc;pc=pc2+1;}
	  else {printf("Stack-Overflow ! PC=%d\n",pc); batch=0;}
	  oldbatch=batch;batch=1;
	  programmlauf();
	  batch=min(oldbatch,batch);
	  if(osp!=sp) {
	    pc=stack[--sp]; /* wenn error innerhalb der func. */
            puts("Error within FUNCTION.");
	  }
	}
	free(buffer);
	return(returnvalue.f);
      }

    free(buffer);
    return(0.0);
 }

 /* loese die Parameterliste auf und weise die Werte auf die neuen lokalen
    Variablen zu */

int do_parameterliste(char *pos, char *pos2) {
  char w3[strlen(pos)+1],w4[strlen(pos)+1];
  char w5[strlen(pos2)+1],w6[strlen(pos2)+1];
  int e1,e2;
 /* printf("GOSUB: <%s> <%s>\n",pos,pos2);*/
  e1=wort_sep(pos,',',TRUE,w3,w4);
  e2=wort_sep(pos2,',',TRUE,w5,w6);
  sp++;  /* Uebergabeparameter sind lokal ! */
  while(e1 && e2) {
  /*  printf("ZU: %s=%s\n",w3,w5); */
    c_dolocal(w5,w3);
    e1=wort_sep(w4,',',TRUE,w3,w4);
    e2=wort_sep(w6,',',TRUE,w5,w6);
  }
  sp--;
  return((e1!=e2) ? 1 : 0);
}


STRING do_sfunktion(char *name,char *argumente) {
  char *buffer,*pos;
  int pc2;

  buffer=malloc(strlen(name)+1);
  strcpy(buffer,name);
  pos=argumente;
  pc2=procnr(buffer,2);
  if(pc2!=-1) {
	if(do_parameterliste(pos,procs[pc2].parameterliste)) xberror(42,buffer); /* Zu wenig Parameter */
	else {
	  int oldbatch,osp=sp;
	
	  pc2=procs[pc2].zeile;
	  if(sp<STACKSIZE) {stack[sp++]=pc;pc=pc2+1;}
	  else {printf("Stack-Overflow ! PC=%d\n",pc); batch=0;}
	  oldbatch=batch;batch=1;
	  programmlauf();
	  batch=min(oldbatch,batch);
	  if(osp!=sp) {
	    pc=stack[--sp]; /* wenn error innerhalb der func. */
            puts("Error within FUNCTION.");
	  }
	}
    free(buffer);
    return(returnvalue.str);
  }
  xberror(44,buffer); /* Funktion  nicht definiert */
  free(buffer);
  return(create_string(NULL));
}