/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "HomeLogic.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int yylex(void);
void yyerror(const char *s);
extern FILE *yyin;
extern int yylineno;

/* Part: Table of Names */
#define MAX_ENTRIES 100
#define MAX_SCHEDULES   50

enum { K_DEVICE, K_SENSOR, K_VAR, K_ROUTINE };
const char *kindLabel[] = { "DEVICE", "SENSOR", "VARIABLE", "ROUTINE" };

typedef struct Node Node;

typedef struct
{
    char name[50];
    int  kind;
    int  value;     /* SENSOR reading / VARIABLE value        */
    int  state;     /* DEVICE: 1 = ON, 0 = OFF                */
    int  level;     /* DEVICE: brightness / speed (0 to 100)  */
    Node *body;     /* ROUTINE: its statements                */
} Entry;

Entry homeTable[MAX_ENTRIES];
int entryCount = 0;
int errors = 0;
Node *root = NULL;

/* Part: Syntax Tree */
enum { N_NUM, N_VAR, N_BIN, N_NEG, N_NOT, N_ASSIGN, N_ON, N_OFF, N_DIM,
       N_SENSE, N_SHOWS, N_SHOWE, N_IF, N_REPEAT, N_RUN, N_AT, N_CLOCK, N_LIST,
       N_INPUT };

struct Node
{
    int   type;
    int   val;      /* number, symbol index or time              */
    int   op;       /* operator code for N_BIN                   */
    char *str;      /* text for SHOW "..."                       */
    Node *a, *b, *c;
};

Node *newNode(int type, int val, Node *a, Node *b, Node *c)
{
    Node *n = (Node *)calloc(1, sizeof(Node));
    n->type = type; n->val = val;
    n->a = a; n->b = b; n->c = c;
    return n;
}

Node *makeBinary(int op, Node *l, Node *r)
{
    Node *n = newNode(N_BIN, 0, l, r, NULL);
    n->op = op;
    return n;
}

/* operator codes used inside N_BIN */
#define OP_EQ 'e'
#define OP_NE 'n'
#define OP_LE 'l'
#define OP_GE 'g'
#define OP_AND '&'
#define OP_OR  '|'

/* Part: Semantic Helpers */
void reportSemantic(const char *msg, const char *name)
{
    printf("HomeLogic semantic error, line %d: ", yylineno);
    printf(msg, name);
    printf("\n");
    errors++;
}

int lookupName(const char *name)
{
    int i;
    for (i = 0; i < entryCount; i++)
        if (strcmp(homeTable[i].name, name) == 0)
            return i;
    return -1;
}

int registerName(const char *name, int kind)
{
    if (lookupName(name) != -1)
    {
        reportSemantic("'%s' is already declared.", name);
        return -1;
    }
    if (entryCount >= MAX_ENTRIES)
    {
        reportSemantic("Home table is full (while declaring '%s').", name);
        return -1;
    }
    strcpy(homeTable[entryCount].name, name);
    homeTable[entryCount].kind  = kind;
    homeTable[entryCount].value = 0;
    homeTable[entryCount].state = 0;
    homeTable[entryCount].level = 0;
    homeTable[entryCount].body  = NULL;
    printf("[Added to home] %s '%s'\n", kindLabel[kind], name);
    return entryCount++;
}

/* find a symbol that must be of the given kind (-1 means "any value kind") */
int requireKind(const char *name, int kind)
{
    int i = lookupName(name);
    if (i == -1)
    {
        reportSemantic("'%s' is not declared.", name);
        return -1;
    }
    if (homeTable[i].kind != kind)
    {
        printf("HomeLogic semantic error, line %d: '%s' is a %s, but a %s is required here.\n",
               yylineno, name, kindLabel[homeTable[i].kind], kindLabel[kind]);
        errors++;
        return -1;
    }
    return i;
}

/* symbol used inside an expression: DEVICE, SENSOR or VARIABLE */
int valueIndex(const char *name)
{
    int i = lookupName(name);
    if (i == -1)
    {
        reportSemantic("'%s' is not declared.", name);
        return -1;
    }
    if (homeTable[i].kind == K_ROUTINE)
    {
        reportSemantic("ROUTINE '%s' cannot be used inside an expression.", name);
        return -1;
    }
    return i;
}

int validTime(int t)
{
    int ok = (t / 100 < 24) && (t % 100 < 60);
    if (!ok)
    {
        printf("HomeLogic semantic error, line %d: Invalid time %02d:%02d (use 00:00 to 23:59).\n",
               yylineno, t / 100, t % 100);
        errors++;
    }
    return ok;
}

/* Part: Runtime Input Validation */
int readValidInt(const char *name, const char *kind)
{
    char buf[256];
    int attempts = 0;
    while (attempts < 3)
    {
        printf("Enter %s value for %s: ", kind, name);
        fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            printf("  [Invalid] Could not read input.\n");
            attempts++;
            continue;
        }
        buf[strcspn(buf, "\n")] = 0;
        if (strlen(buf) == 0)
        {
            printf("  [Invalid] Empty input. Please enter an integer.\n");
            attempts++;
            continue;
        }
        char *endptr;
        long val = strtol(buf, &endptr, 10);
        if (*endptr != '\0')
        {
            printf("  [Invalid] '%s' is not a valid integer.\n", buf);
            attempts++;
            continue;
        }
        if (val < -99999 || val > 99999)
        {
            printf("  [Invalid] %ld is out of range (-99999 to 99999).\n", val);
            attempts++;
            continue;
        }
        printf("  [OK] %s = %d\n", name, (int)val);
        return (int)val;
    }
    printf("  [Warning] No valid input after %d attempts; defaulting to 0.\n", attempts);
    return 0;
}

/* Part: Execution (Simulator) */
int virtualClock = 0;       /* virtual clock as HHMM */
int firingNow = 0;          /* 1 while an AT rule is running */

struct { int time; Node *body; } scheduleList[MAX_SCHEDULES];
int scheduleCount = 0;

void tick(void)
{
    printf("[%02d:%02d] ", virtualClock / 100, virtualClock % 100);
}

int calcNode(Node *n)
{
    int l, r;
    if (!n) return 0;
    switch (n->type)
    {
        case N_NUM: return n->val;
        case N_VAR:
            if (homeTable[n->val].kind == K_DEVICE)
                return homeTable[n->val].state;
            return homeTable[n->val].value;
        case N_NEG: return -calcNode(n->a);
        case N_NOT: return !calcNode(n->a);
        case N_BIN:
            l = calcNode(n->a);
            r = calcNode(n->b);
            switch (n->op)
            {
                case '+': return l + r;
                case '-': return l - r;
                case '*': return l * r;
                case '/':
                    if (r == 0)
                    {
                        printf("Runtime problem: Division by zero.\n");
                        return 0;
                    }
                    return l / r;
                case '<': return l < r;
                case '>': return l > r;
                case OP_EQ: return l == r;
                case OP_NE: return l != r;
                case OP_LE: return l <= r;
                case OP_GE: return l >= r;
                case OP_AND: return l && r;
                case OP_OR:  return l || r;
            }
    }
    return 0;
}

void runNode(Node *n)
{
    int i, count, k;
    if (!n) return;
    switch (n->type)
    {
        case N_LIST:
            runNode(n->a);
            runNode(n->b);
            break;

        case N_ASSIGN:
            homeTable[n->val].value = calcNode(n->a);
            break;

        case N_ON:
            tick();
            if (homeTable[n->val].state)
                printf("%s is already ON.\n", homeTable[n->val].name);
            else
            {
                homeTable[n->val].state = 1;
                if (homeTable[n->val].level == 0) homeTable[n->val].level = 100;
                printf("%s -> ON\n", homeTable[n->val].name);
            }
            break;

        case N_OFF:
            tick();
            if (!homeTable[n->val].state)
                printf("%s is already OFF.\n", homeTable[n->val].name);
            else
            {
                homeTable[n->val].state = 0;
                printf("%s -> OFF\n", homeTable[n->val].name);
            }
            break;

        case N_DIM:
            k = calcNode(n->a);
            if (k < 0 || k > 100)
            {
                printf("Runtime problem: DIM level must be 0 to 100 (got %d).\n", k);
                break;
            }
            homeTable[n->val].level = k;
            homeTable[n->val].state = (k > 0);
            tick();
            printf("%s level set to %d%%\n", homeTable[n->val].name, k);
            break;

        case N_SENSE:
            homeTable[n->val].value = readValidInt(homeTable[n->val].name, "sensor");
            break;

        case N_INPUT:
            homeTable[n->val].value = readValidInt(homeTable[n->val].name, "variable");
            break;

        case N_SHOWS:
            tick();
            printf("%s\n", n->str);
            break;

        case N_SHOWE:
            tick();
            printf("%d\n", calcNode(n->a));
            break;

        case N_IF:
            if (calcNode(n->a)) runNode(n->b);
            else            runNode(n->c);
            break;

        case N_REPEAT:
            count = calcNode(n->a);
            for (i = 0; i < count && i < 1000; i++)
                runNode(n->b);
            break;

        case N_RUN:
            tick();
            printf("Running routine %s\n", homeTable[n->val].name);
            runNode(homeTable[n->val].body);
            break;

        case N_AT:
            if (scheduleCount >= MAX_SCHEDULES)
                printf("Runtime problem: Too many AT rules.\n");
            else
            {
                scheduleList[scheduleCount].time = n->val;
                scheduleList[scheduleCount].body = n->a;
                scheduleCount++;
                printf("[Scheduled] rule for %02d:%02d stored.\n", n->val / 100, n->val % 100);
            }
            break;

        case N_CLOCK:
            if (firingNow)
            {
                printf("Runtime problem: CLOCK cannot be used inside an AT rule.\n");
                break;
            }
            virtualClock = n->val;
            tick();
            printf("Clock moved to %02d:%02d\n", n->val / 100, n->val % 100);
            firingNow = 1;
            for (i = 0; i < scheduleCount; i++)
                if (scheduleList[i].time == virtualClock)
                {
                    tick();
                    printf("schedule fired\n");
                    runNode(scheduleList[i].body);
                }
            firingNow = 0;
            break;
    }
}

void printHomeTable(void)
{
    int i;
    printf("\n---- Final state of every name ----\n");
    printf("%-16s %-10s %s\n", "NAME", "TYPE", "NOW");
    for (i = 0; i < entryCount; i++)
    {
        printf("%-16s %-10s ", homeTable[i].name, kindLabel[homeTable[i].kind]);
        switch (homeTable[i].kind)
        {
            case K_DEVICE:
                printf("%s (level %d%%)\n", homeTable[i].state ? "ON" : "OFF", homeTable[i].level);
                break;
            case K_ROUTINE:
                printf("defined\n");
                break;
            default:
                printf("%d\n", homeTable[i].value);
        }
    }
}

#line 471 "HomeLogic.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "HomeLogic.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_DEVICE = 3,                     /* DEVICE  */
  YYSYMBOL_SENSOR = 4,                     /* SENSOR  */
  YYSYMBOL_VAR = 5,                        /* VAR  */
  YYSYMBOL_SWITCHON = 6,                   /* SWITCHON  */
  YYSYMBOL_SWITCHOFF = 7,                  /* SWITCHOFF  */
  YYSYMBOL_DIM = 8,                        /* DIM  */
  YYSYMBOL_SENSE = 9,                      /* SENSE  */
  YYSYMBOL_INPUT = 10,                     /* INPUT  */
  YYSYMBOL_SHOW = 11,                      /* SHOW  */
  YYSYMBOL_WHEN = 12,                      /* WHEN  */
  YYSYMBOL_OTHERWISE = 13,                 /* OTHERWISE  */
  YYSYMBOL_REPEAT = 14,                    /* REPEAT  */
  YYSYMBOL_ROUTINE = 15,                   /* ROUTINE  */
  YYSYMBOL_RUN = 16,                       /* RUN  */
  YYSYMBOL_AT = 17,                        /* AT  */
  YYSYMBOL_CLOCK = 18,                     /* CLOCK  */
  YYSYMBOL_EQ = 19,                        /* EQ  */
  YYSYMBOL_NE = 20,                        /* NE  */
  YYSYMBOL_LE = 21,                        /* LE  */
  YYSYMBOL_GE = 22,                        /* GE  */
  YYSYMBOL_NUMBER = 23,                    /* NUMBER  */
  YYSYMBOL_TIME = 24,                      /* TIME  */
  YYSYMBOL_ID = 25,                        /* ID  */
  YYSYMBOL_STRING = 26,                    /* STRING  */
  YYSYMBOL_OR = 27,                        /* OR  */
  YYSYMBOL_AND = 28,                       /* AND  */
  YYSYMBOL_NOT = 29,                       /* NOT  */
  YYSYMBOL_30_ = 30,                       /* '<'  */
  YYSYMBOL_31_ = 31,                       /* '>'  */
  YYSYMBOL_32_ = 32,                       /* '+'  */
  YYSYMBOL_33_ = 33,                       /* '-'  */
  YYSYMBOL_34_ = 34,                       /* '*'  */
  YYSYMBOL_35_ = 35,                       /* '/'  */
  YYSYMBOL_UMINUS = 36,                    /* UMINUS  */
  YYSYMBOL_37_ = 37,                       /* ';'  */
  YYSYMBOL_38_ = 38,                       /* '='  */
  YYSYMBOL_39_ = 39,                       /* '('  */
  YYSYMBOL_40_ = 40,                       /* ')'  */
  YYSYMBOL_41_ = 41,                       /* '{'  */
  YYSYMBOL_42_ = 42,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 43,                  /* $accept  */
  YYSYMBOL_program = 44,                   /* program  */
  YYSYMBOL_statements = 45,                /* statements  */
  YYSYMBOL_statement = 46,                 /* statement  */
  YYSYMBOL_declaration = 47,               /* declaration  */
  YYSYMBOL_assignment = 48,                /* assignment  */
  YYSYMBOL_action = 49,                    /* action  */
  YYSYMBOL_control = 50,                   /* control  */
  YYSYMBOL_block = 51,                     /* block  */
  YYSYMBOL_expression = 52                 /* expression  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  48
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   331

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  10
/* YYNRULES -- Number of rules.  */
#define YYNRULES  48
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  111

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   285


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      39,    40,    34,    32,     2,    33,     2,    35,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    37,
      30,    38,    31,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    41,     2,    42,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    36
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   435,   435,   442,   446,   453,   454,   455,   456,   457,
     461,   467,   473,   479,   485,   491,   501,   521,   527,   533,
     539,   545,   562,   567,   571,   577,   584,   588,   592,   596,
     603,   604,   608,   609,   610,   611,   612,   613,   614,   615,
     616,   617,   618,   619,   620,   621,   622,   623,   624
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "DEVICE", "SENSOR",
  "VAR", "SWITCHON", "SWITCHOFF", "DIM", "SENSE", "INPUT", "SHOW", "WHEN",
  "OTHERWISE", "REPEAT", "ROUTINE", "RUN", "AT", "CLOCK", "EQ", "NE", "LE",
  "GE", "NUMBER", "TIME", "ID", "STRING", "OR", "AND", "NOT", "'<'", "'>'",
  "'+'", "'-'", "'*'", "'/'", "UMINUS", "';'", "'='", "'('", "')'", "'{'",
  "'}'", "$accept", "program", "statements", "statement", "declaration",
  "assignment", "action", "control", "block", "expression", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-43)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-3)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     128,   -35,    23,    24,    27,    28,    30,    32,    36,    37,
     292,    26,    21,    38,    41,    56,    57,    44,    95,   105,
     -43,   -43,   -43,   -43,   -43,   -43,    64,   -33,   -27,    65,
      66,    21,    67,    81,   -43,   -43,    88,    21,    21,    21,
     -13,    21,   129,    85,    90,    85,    91,    21,   -43,   -43,
     -43,   -43,    21,   -43,    21,   -43,   -43,   180,   -43,   -43,
     -43,   273,   -43,   146,    21,    21,    21,    21,    21,    21,
      21,    21,    21,    21,    21,    21,   -43,   163,    22,   -43,
     -43,   -43,   -43,   -43,   199,   218,   237,   -43,   -43,   279,
     279,   279,   279,   256,   273,   279,   279,   -22,   -22,   -43,
     -43,    85,   -43,    82,   -43,   -43,   -43,   134,   -43,    85,
     -43
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,     0,     0,     0,    47,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     1,     3,
      10,    11,     0,    13,     0,    17,    18,     0,    20,    21,
      22,    44,    45,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    23,     0,     0,    28,
      15,    24,    29,    25,     0,     0,     0,    19,    46,    38,
      39,    40,    41,    43,    42,    36,    37,    32,    33,    34,
      35,     0,    31,     0,    16,    12,    14,    26,    30,     0,
      27
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -43,   -43,    63,   -19,   -43,   -43,   -43,   -43,   -42,     4
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,    18,    19,    20,    21,    22,    23,    24,    79,    40
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      49,    80,    25,    82,    51,    52,    64,    65,    66,    67,
      53,    54,    74,    75,    68,    69,    42,    70,    71,    72,
      73,    74,    75,     1,    76,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    57,    12,    13,    14,    15,
      16,    61,    62,    63,    34,    77,    35,    17,    26,    27,
      37,    84,    28,    29,    38,    30,    85,    31,    86,   107,
      39,    32,    33,    43,   102,    41,    44,   110,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
      45,    46,    47,     1,    49,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    48,    12,    13,    14,    15,
      16,    50,    55,    56,    58,    -2,     1,    17,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    59,    12,
      13,    14,    15,    16,   108,    60,    78,    81,    83,     1,
      17,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,   103,    12,    13,    14,    15,    16,   109,    64,    65,
      66,    67,     0,    17,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,    75,    64,    65,    66,    67,     0,
      78,     0,     0,    68,    69,     0,    70,    71,    72,    73,
      74,    75,    64,    65,    66,    67,    88,     0,     0,     0,
      68,    69,     0,    70,    71,    72,    73,    74,    75,    64,
      65,    66,    67,   101,     0,     0,     0,    68,    69,     0,
      70,    71,    72,    73,    74,    75,     0,    87,    64,    65,
      66,    67,     0,     0,     0,     0,    68,    69,     0,    70,
      71,    72,    73,    74,    75,     0,   104,    64,    65,    66,
      67,     0,     0,     0,     0,    68,    69,     0,    70,    71,
      72,    73,    74,    75,     0,   105,    64,    65,    66,    67,
       0,     0,     0,     0,    68,    69,     0,    70,    71,    72,
      73,    74,    75,     0,   106,    64,    65,    66,    67,     0,
       0,     0,     0,     0,    69,     0,    70,    71,    72,    73,
      74,    75,    64,    65,    66,    67,     0,     0,    -3,    -3,
      -3,    -3,     0,    70,    71,    72,    73,    74,    75,    -3,
      -3,    72,    73,    74,    75,    34,     0,    35,    36,     0,
       0,    37,     0,     0,     0,    38,     0,     0,     0,     0,
       0,    39
};

static const yytype_int8 yycheck[] =
{
      19,    43,    37,    45,    37,    38,    19,    20,    21,    22,
      37,    38,    34,    35,    27,    28,    12,    30,    31,    32,
      33,    34,    35,     1,    37,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    31,    14,    15,    16,    17,
      18,    37,    38,    39,    23,    41,    25,    25,    25,    25,
      29,    47,    25,    25,    33,    25,    52,    25,    54,   101,
      39,    25,    25,    25,    42,    39,    25,   109,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      24,    24,    38,     1,   103,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,     0,    14,    15,    16,    17,
      18,    37,    37,    37,    37,     0,     1,    25,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    37,    14,
      15,    16,    17,    18,    42,    37,    41,    37,    37,     1,
      25,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    78,    14,    15,    16,    17,    18,    13,    19,    20,
      21,    22,    -1,    25,    -1,    -1,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    19,    20,    21,    22,    -1,
      41,    -1,    -1,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    19,    20,    21,    22,    40,    -1,    -1,    -1,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    19,
      20,    21,    22,    40,    -1,    -1,    -1,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    -1,    37,    19,    20,
      21,    22,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    -1,    37,    19,    20,    21,
      22,    -1,    -1,    -1,    -1,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    -1,    37,    19,    20,    21,    22,
      -1,    -1,    -1,    -1,    27,    28,    -1,    30,    31,    32,
      33,    34,    35,    -1,    37,    19,    20,    21,    22,    -1,
      -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,    33,
      34,    35,    19,    20,    21,    22,    -1,    -1,    19,    20,
      21,    22,    -1,    30,    31,    32,    33,    34,    35,    30,
      31,    32,    33,    34,    35,    23,    -1,    25,    26,    -1,
      -1,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    39
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    14,    15,    16,    17,    18,    25,    44,    45,
      46,    47,    48,    49,    50,    37,    25,    25,    25,    25,
      25,    25,    25,    25,    23,    25,    26,    29,    33,    39,
      52,    39,    52,    25,    25,    24,    24,    38,     0,    46,
      37,    37,    38,    37,    38,    37,    37,    52,    37,    37,
      37,    52,    52,    52,    19,    20,    21,    22,    27,    28,
      30,    31,    32,    33,    34,    35,    37,    52,    41,    51,
      51,    37,    51,    37,    52,    52,    52,    37,    40,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    40,    42,    45,    37,    37,    37,    51,    42,    13,
      51
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    43,    44,    45,    45,    46,    46,    46,    46,    46,
      47,    47,    47,    47,    47,    47,    48,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    50,    50,    50,    50,
      51,    51,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     1,     1,     2,
       3,     3,     5,     3,     5,     3,     4,     3,     3,     4,
       3,     3,     3,     3,     3,     3,     5,     7,     3,     3,
       3,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     2,     3,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: statements  */
#line 436 "HomeLogic.y"
        {
            root = (yyvsp[0].node);
        }
#line 1887 "HomeLogic.tab.c"
    break;

  case 3: /* statements: statements statement  */
#line 443 "HomeLogic.y"
        {
            (yyval.node) = newNode(N_LIST, 0, (yyvsp[-1].node), (yyvsp[0].node), NULL);
        }
#line 1895 "HomeLogic.tab.c"
    break;

  case 4: /* statements: statement  */
#line 447 "HomeLogic.y"
        {
            (yyval.node) = (yyvsp[0].node);
        }
#line 1903 "HomeLogic.tab.c"
    break;

  case 5: /* statement: declaration  */
#line 453 "HomeLogic.y"
                        { (yyval.node) = (yyvsp[0].node); }
#line 1909 "HomeLogic.tab.c"
    break;

  case 6: /* statement: assignment  */
#line 454 "HomeLogic.y"
                        { (yyval.node) = (yyvsp[0].node); }
#line 1915 "HomeLogic.tab.c"
    break;

  case 7: /* statement: action  */
#line 455 "HomeLogic.y"
                        { (yyval.node) = (yyvsp[0].node); }
#line 1921 "HomeLogic.tab.c"
    break;

  case 8: /* statement: control  */
#line 456 "HomeLogic.y"
                        { (yyval.node) = (yyvsp[0].node); }
#line 1927 "HomeLogic.tab.c"
    break;

  case 9: /* statement: error ';'  */
#line 457 "HomeLogic.y"
                        { (yyval.node) = NULL; yyerrok; }
#line 1933 "HomeLogic.tab.c"
    break;

  case 10: /* declaration: DEVICE ID ';'  */
#line 462 "HomeLogic.y"
        {
            registerName((yyvsp[-1].str), K_DEVICE);
            free((yyvsp[-1].str));
            (yyval.node) = NULL;
        }
#line 1943 "HomeLogic.tab.c"
    break;

  case 11: /* declaration: SENSOR ID ';'  */
#line 468 "HomeLogic.y"
        {
            registerName((yyvsp[-1].str), K_SENSOR);
            free((yyvsp[-1].str));
            (yyval.node) = NULL;
        }
#line 1953 "HomeLogic.tab.c"
    break;

  case 12: /* declaration: SENSOR ID '=' expression ';'  */
#line 474 "HomeLogic.y"
        {
            int i = registerName((yyvsp[-3].str), K_SENSOR);
            (yyval.node) = (i >= 0) ? newNode(N_ASSIGN, i, (yyvsp[-1].node), NULL, NULL) : NULL;
            free((yyvsp[-3].str));
        }
#line 1963 "HomeLogic.tab.c"
    break;

  case 13: /* declaration: VAR ID ';'  */
#line 480 "HomeLogic.y"
        {
            registerName((yyvsp[-1].str), K_VAR);
            free((yyvsp[-1].str));
            (yyval.node) = NULL;
        }
#line 1973 "HomeLogic.tab.c"
    break;

  case 14: /* declaration: VAR ID '=' expression ';'  */
#line 486 "HomeLogic.y"
        {
            int i = registerName((yyvsp[-3].str), K_VAR);
            (yyval.node) = (i >= 0) ? newNode(N_ASSIGN, i, (yyvsp[-1].node), NULL, NULL) : NULL;
            free((yyvsp[-3].str));
        }
#line 1983 "HomeLogic.tab.c"
    break;

  case 15: /* declaration: ROUTINE ID block  */
#line 492 "HomeLogic.y"
        {
            int i = registerName((yyvsp[-1].str), K_ROUTINE);
            if (i >= 0) homeTable[i].body = (yyvsp[0].node);
            free((yyvsp[-1].str));
            (yyval.node) = NULL;
        }
#line 1994 "HomeLogic.tab.c"
    break;

  case 16: /* assignment: ID '=' expression ';'  */
#line 502 "HomeLogic.y"
        {
            int i = lookupName((yyvsp[-3].str));
            if (i == -1)
            {
                reportSemantic("'%s' is not declared.", (yyvsp[-3].str));
                (yyval.node) = NULL;
            }
            else if (homeTable[i].kind == K_VAR || homeTable[i].kind == K_SENSOR)
                (yyval.node) = newNode(N_ASSIGN, i, (yyvsp[-1].node), NULL, NULL);
            else
            {
                reportSemantic("'%s' is a DEVICE/ROUTINE; use SWITCHON, SWITCHOFF or DIM instead of '='.", (yyvsp[-3].str));
                (yyval.node) = NULL;
            }
            free((yyvsp[-3].str));
        }
#line 2015 "HomeLogic.tab.c"
    break;

  case 17: /* action: SWITCHON ID ';'  */
#line 522 "HomeLogic.y"
        {
            int i = requireKind((yyvsp[-1].str), K_DEVICE);
            (yyval.node) = (i >= 0) ? newNode(N_ON, i, NULL, NULL, NULL) : NULL;
            free((yyvsp[-1].str));
        }
#line 2025 "HomeLogic.tab.c"
    break;

  case 18: /* action: SWITCHOFF ID ';'  */
#line 528 "HomeLogic.y"
        {
            int i = requireKind((yyvsp[-1].str), K_DEVICE);
            (yyval.node) = (i >= 0) ? newNode(N_OFF, i, NULL, NULL, NULL) : NULL;
            free((yyvsp[-1].str));
        }
#line 2035 "HomeLogic.tab.c"
    break;

  case 19: /* action: DIM ID expression ';'  */
#line 534 "HomeLogic.y"
        {
            int i = requireKind((yyvsp[-2].str), K_DEVICE);
            (yyval.node) = (i >= 0) ? newNode(N_DIM, i, (yyvsp[-1].node), NULL, NULL) : NULL;
            free((yyvsp[-2].str));
        }
#line 2045 "HomeLogic.tab.c"
    break;

  case 20: /* action: SENSE ID ';'  */
#line 540 "HomeLogic.y"
        {
            int i = requireKind((yyvsp[-1].str), K_SENSOR);
            (yyval.node) = (i >= 0) ? newNode(N_SENSE, i, NULL, NULL, NULL) : NULL;
            free((yyvsp[-1].str));
        }
#line 2055 "HomeLogic.tab.c"
    break;

  case 21: /* action: INPUT ID ';'  */
#line 546 "HomeLogic.y"
        {
            int i = lookupName((yyvsp[-1].str));
            if (i == -1)
            {
                reportSemantic("'%s' is not declared.", (yyvsp[-1].str));
                (yyval.node) = NULL;
            }
            else if (homeTable[i].kind == K_VAR || homeTable[i].kind == K_SENSOR)
                (yyval.node) = newNode(N_INPUT, i, NULL, NULL, NULL);
            else
            {
                reportSemantic("INPUT can only read into a VARIABLE or SENSOR, not '%s'.", (yyvsp[-1].str));
                (yyval.node) = NULL;
            }
            free((yyvsp[-1].str));
        }
#line 2076 "HomeLogic.tab.c"
    break;

  case 22: /* action: SHOW STRING ';'  */
#line 563 "HomeLogic.y"
        {
            (yyval.node) = newNode(N_SHOWS, 0, NULL, NULL, NULL);
            (yyval.node)->str = (yyvsp[-1].str);
        }
#line 2085 "HomeLogic.tab.c"
    break;

  case 23: /* action: SHOW expression ';'  */
#line 568 "HomeLogic.y"
        {
            (yyval.node) = newNode(N_SHOWE, 0, (yyvsp[-1].node), NULL, NULL);
        }
#line 2093 "HomeLogic.tab.c"
    break;

  case 24: /* action: RUN ID ';'  */
#line 572 "HomeLogic.y"
        {
            int i = requireKind((yyvsp[-1].str), K_ROUTINE);
            (yyval.node) = (i >= 0) ? newNode(N_RUN, i, NULL, NULL, NULL) : NULL;
            free((yyvsp[-1].str));
        }
#line 2103 "HomeLogic.tab.c"
    break;

  case 25: /* action: CLOCK TIME ';'  */
#line 578 "HomeLogic.y"
        {
            (yyval.node) = validTime((yyvsp[-1].num)) ? newNode(N_CLOCK, (yyvsp[-1].num), NULL, NULL, NULL) : NULL;
        }
#line 2111 "HomeLogic.tab.c"
    break;

  case 26: /* control: WHEN '(' expression ')' block  */
#line 585 "HomeLogic.y"
        {
            (yyval.node) = newNode(N_IF, 0, (yyvsp[-2].node), (yyvsp[0].node), NULL);
        }
#line 2119 "HomeLogic.tab.c"
    break;

  case 27: /* control: WHEN '(' expression ')' block OTHERWISE block  */
#line 589 "HomeLogic.y"
        {
            (yyval.node) = newNode(N_IF, 0, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node));
        }
#line 2127 "HomeLogic.tab.c"
    break;

  case 28: /* control: REPEAT expression block  */
#line 593 "HomeLogic.y"
        {
            (yyval.node) = newNode(N_REPEAT, 0, (yyvsp[-1].node), (yyvsp[0].node), NULL);
        }
#line 2135 "HomeLogic.tab.c"
    break;

  case 29: /* control: AT TIME block  */
#line 597 "HomeLogic.y"
        {
            (yyval.node) = validTime((yyvsp[-1].num)) ? newNode(N_AT, (yyvsp[-1].num), (yyvsp[0].node), NULL, NULL) : NULL;
        }
#line 2143 "HomeLogic.tab.c"
    break;

  case 30: /* block: '{' statements '}'  */
#line 603 "HomeLogic.y"
                            { (yyval.node) = (yyvsp[-1].node); }
#line 2149 "HomeLogic.tab.c"
    break;

  case 31: /* block: '{' '}'  */
#line 604 "HomeLogic.y"
                            { (yyval.node) = NULL; }
#line 2155 "HomeLogic.tab.c"
    break;

  case 32: /* expression: expression '+' expression  */
#line 608 "HomeLogic.y"
                                    { (yyval.node) = makeBinary('+', (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2161 "HomeLogic.tab.c"
    break;

  case 33: /* expression: expression '-' expression  */
#line 609 "HomeLogic.y"
                                    { (yyval.node) = makeBinary('-', (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2167 "HomeLogic.tab.c"
    break;

  case 34: /* expression: expression '*' expression  */
#line 610 "HomeLogic.y"
                                    { (yyval.node) = makeBinary('*', (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2173 "HomeLogic.tab.c"
    break;

  case 35: /* expression: expression '/' expression  */
#line 611 "HomeLogic.y"
                                    { (yyval.node) = makeBinary('/', (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2179 "HomeLogic.tab.c"
    break;

  case 36: /* expression: expression '<' expression  */
#line 612 "HomeLogic.y"
                                    { (yyval.node) = makeBinary('<', (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2185 "HomeLogic.tab.c"
    break;

  case 37: /* expression: expression '>' expression  */
#line 613 "HomeLogic.y"
                                    { (yyval.node) = makeBinary('>', (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2191 "HomeLogic.tab.c"
    break;

  case 38: /* expression: expression EQ expression  */
#line 614 "HomeLogic.y"
                                    { (yyval.node) = makeBinary(OP_EQ, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2197 "HomeLogic.tab.c"
    break;

  case 39: /* expression: expression NE expression  */
#line 615 "HomeLogic.y"
                                    { (yyval.node) = makeBinary(OP_NE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2203 "HomeLogic.tab.c"
    break;

  case 40: /* expression: expression LE expression  */
#line 616 "HomeLogic.y"
                                    { (yyval.node) = makeBinary(OP_LE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2209 "HomeLogic.tab.c"
    break;

  case 41: /* expression: expression GE expression  */
#line 617 "HomeLogic.y"
                                    { (yyval.node) = makeBinary(OP_GE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2215 "HomeLogic.tab.c"
    break;

  case 42: /* expression: expression AND expression  */
#line 618 "HomeLogic.y"
                                    { (yyval.node) = makeBinary(OP_AND, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2221 "HomeLogic.tab.c"
    break;

  case 43: /* expression: expression OR expression  */
#line 619 "HomeLogic.y"
                                    { (yyval.node) = makeBinary(OP_OR, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2227 "HomeLogic.tab.c"
    break;

  case 44: /* expression: NOT expression  */
#line 620 "HomeLogic.y"
                                    { (yyval.node) = newNode(N_NOT, 0, (yyvsp[0].node), NULL, NULL); }
#line 2233 "HomeLogic.tab.c"
    break;

  case 45: /* expression: '-' expression  */
#line 621 "HomeLogic.y"
                                      { (yyval.node) = newNode(N_NEG, 0, (yyvsp[0].node), NULL, NULL); }
#line 2239 "HomeLogic.tab.c"
    break;

  case 46: /* expression: '(' expression ')'  */
#line 622 "HomeLogic.y"
                                    { (yyval.node) = (yyvsp[-1].node); }
#line 2245 "HomeLogic.tab.c"
    break;

  case 47: /* expression: NUMBER  */
#line 623 "HomeLogic.y"
                                    { (yyval.node) = newNode(N_NUM, (yyvsp[0].num), NULL, NULL, NULL); }
#line 2251 "HomeLogic.tab.c"
    break;

  case 48: /* expression: ID  */
#line 625 "HomeLogic.y"
        {
            int i = valueIndex((yyvsp[0].str));
            (yyval.node) = (i >= 0) ? newNode(N_VAR, i, NULL, NULL, NULL) : newNode(N_NUM, 0, NULL, NULL, NULL);
            free((yyvsp[0].str));
        }
#line 2261 "HomeLogic.tab.c"
    break;


#line 2265 "HomeLogic.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 632 "HomeLogic.y"


/* Part: C Functions */
void yyerror(const char *s)
{
    printf("HomeLogic syntax error, line %d -> %s\n", yylineno, s);
    errors++;
}

int main(int argc, char *argv[])
{
    printf("========================================\n");
    printf("        HomeLogic  v1.0  (.home)        \n");
    printf("========================================\n\n");

    if (argc > 1)
    {
        yyin = fopen(argv[1], "r");
        if (yyin == NULL)
        {
            printf("Cannot open file: %s\n", argv[1]);
            return 1;
        }
    }

    printf(">> Checking program\n");
    yyparse();

    if (errors > 0)
    {
        printf("\nBuild stopped: %d error(s) found, nothing was executed.\n", errors);
        return 1;
    }

    printf("\n>> Starting the virtual home\n");
    runNode(root);
    printHomeTable();
    printf("\nSimulation complete.\n");

    if (argc > 1)
        fclose(yyin);
    return 0;
}
