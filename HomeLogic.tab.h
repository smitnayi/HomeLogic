/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_HOMELOGIC_TAB_H_INCLUDED
# define YY_YY_HOMELOGIC_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    DEVICE = 258,                  /* DEVICE  */
    SENSOR = 259,                  /* SENSOR  */
    VAR = 260,                     /* VAR  */
    SWITCHON = 261,                /* SWITCHON  */
    SWITCHOFF = 262,               /* SWITCHOFF  */
    DIM = 263,                     /* DIM  */
    SENSE = 264,                   /* SENSE  */
    INPUT = 265,                   /* INPUT  */
    SHOW = 266,                    /* SHOW  */
    WHEN = 267,                    /* WHEN  */
    OTHERWISE = 268,               /* OTHERWISE  */
    REPEAT = 269,                  /* REPEAT  */
    ROUTINE = 270,                 /* ROUTINE  */
    RUN = 271,                     /* RUN  */
    AT = 272,                      /* AT  */
    CLOCK = 273,                   /* CLOCK  */
    EQ = 274,                      /* EQ  */
    NE = 275,                      /* NE  */
    LE = 276,                      /* LE  */
    GE = 277,                      /* GE  */
    NUMBER = 278,                  /* NUMBER  */
    TIME = 279,                    /* TIME  */
    ID = 280,                      /* ID  */
    STRING = 281,                  /* STRING  */
    OR = 282,                      /* OR  */
    AND = 283,                     /* AND  */
    NOT = 284,                     /* NOT  */
    UMINUS = 285                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 403 "HomeLogic.y"

    int num;
    char *str;
    struct Node *node;

#line 100 "HomeLogic.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_HOMELOGIC_TAB_H_INCLUDED  */
