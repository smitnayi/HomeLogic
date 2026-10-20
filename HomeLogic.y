%{
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
%}

/* Part: Yacc Declarations */
%union
{
    int num;
    char *str;
    struct Node *node;
}

%error-verbose

%token DEVICE SENSOR VAR SWITCHON SWITCHOFF DIM SENSE INPUT SHOW
%token WHEN OTHERWISE REPEAT ROUTINE RUN AT CLOCK
%token EQ NE LE GE
%token <num> NUMBER
%token <num> TIME
%token <str> ID
%token <str> STRING

%type <node> program statements statement declaration assignment
%type <node> action control block expression

%left OR
%left AND
%right NOT
%nonassoc EQ NE '<' '>' LE GE
%left '+' '-'
%left '*' '/'
%right UMINUS

%%

/* Part: Grammar */

program:
        statements
        {
            root = $1;
        }
        ;

statements:
        statements statement
        {
            $$ = newNode(N_LIST, 0, $1, $2, NULL);
        }
        | statement
        {
            $$ = $1;
        }
        ;

statement:
        declaration     { $$ = $1; }
        | assignment    { $$ = $1; }
        | action        { $$ = $1; }
        | control       { $$ = $1; }
        | error ';'     { $$ = NULL; yyerrok; }
        ;

declaration:
        DEVICE ID ';'
        {
            registerName($2, K_DEVICE);
            free($2);
            $$ = NULL;
        }
        | SENSOR ID ';'
        {
            registerName($2, K_SENSOR);
            free($2);
            $$ = NULL;
        }
        | SENSOR ID '=' expression ';'
        {
            int i = registerName($2, K_SENSOR);
            $$ = (i >= 0) ? newNode(N_ASSIGN, i, $4, NULL, NULL) : NULL;
            free($2);
        }
        | VAR ID ';'
        {
            registerName($2, K_VAR);
            free($2);
            $$ = NULL;
        }
        | VAR ID '=' expression ';'
        {
            int i = registerName($2, K_VAR);
            $$ = (i >= 0) ? newNode(N_ASSIGN, i, $4, NULL, NULL) : NULL;
            free($2);
        }
        | ROUTINE ID block
        {
            int i = registerName($2, K_ROUTINE);
            if (i >= 0) homeTable[i].body = $3;
            free($2);
            $$ = NULL;
        }
        ;

assignment:
        ID '=' expression ';'
        {
            int i = lookupName($1);
            if (i == -1)
            {
                reportSemantic("'%s' is not declared.", $1);
                $$ = NULL;
            }
            else if (homeTable[i].kind == K_VAR || homeTable[i].kind == K_SENSOR)
                $$ = newNode(N_ASSIGN, i, $3, NULL, NULL);
            else
            {
                reportSemantic("'%s' is a DEVICE/ROUTINE; use SWITCHON, SWITCHOFF or DIM instead of '='.", $1);
                $$ = NULL;
            }
            free($1);
        }
        ;

action:
        SWITCHON ID ';'
        {
            int i = requireKind($2, K_DEVICE);
            $$ = (i >= 0) ? newNode(N_ON, i, NULL, NULL, NULL) : NULL;
            free($2);
        }
        | SWITCHOFF ID ';'
        {
            int i = requireKind($2, K_DEVICE);
            $$ = (i >= 0) ? newNode(N_OFF, i, NULL, NULL, NULL) : NULL;
            free($2);
        }
        | DIM ID expression ';'
        {
            int i = requireKind($2, K_DEVICE);
            $$ = (i >= 0) ? newNode(N_DIM, i, $3, NULL, NULL) : NULL;
            free($2);
        }
        | SENSE ID ';'
        {
            int i = requireKind($2, K_SENSOR);
            $$ = (i >= 0) ? newNode(N_SENSE, i, NULL, NULL, NULL) : NULL;
            free($2);
        }
        | INPUT ID ';'
        {
            int i = lookupName($2);
            if (i == -1)
            {
                reportSemantic("'%s' is not declared.", $2);
                $$ = NULL;
            }
            else if (homeTable[i].kind == K_VAR || homeTable[i].kind == K_SENSOR)
                $$ = newNode(N_INPUT, i, NULL, NULL, NULL);
            else
            {
                reportSemantic("INPUT can only read into a VARIABLE or SENSOR, not '%s'.", $2);
                $$ = NULL;
            }
            free($2);
        }
        | SHOW STRING ';'
        {
            $$ = newNode(N_SHOWS, 0, NULL, NULL, NULL);
            $$->str = $2;
        }
        | SHOW expression ';'
        {
            $$ = newNode(N_SHOWE, 0, $2, NULL, NULL);
        }
        | RUN ID ';'
        {
            int i = requireKind($2, K_ROUTINE);
            $$ = (i >= 0) ? newNode(N_RUN, i, NULL, NULL, NULL) : NULL;
            free($2);
        }
        | CLOCK TIME ';'
        {
            $$ = validTime($2) ? newNode(N_CLOCK, $2, NULL, NULL, NULL) : NULL;
        }
        ;

control:
        WHEN '(' expression ')' block
        {
            $$ = newNode(N_IF, 0, $3, $5, NULL);
        }
        | WHEN '(' expression ')' block OTHERWISE block
        {
            $$ = newNode(N_IF, 0, $3, $5, $7);
        }
        | REPEAT expression block
        {
            $$ = newNode(N_REPEAT, 0, $2, $3, NULL);
        }
        | AT TIME block
        {
            $$ = validTime($2) ? newNode(N_AT, $2, $3, NULL, NULL) : NULL;
        }
        ;

block:
        '{' statements '}'  { $$ = $2; }
        | '{' '}'           { $$ = NULL; }
        ;

expression:
        expression '+' expression   { $$ = makeBinary('+', $1, $3); }
        | expression '-' expression { $$ = makeBinary('-', $1, $3); }
        | expression '*' expression { $$ = makeBinary('*', $1, $3); }
        | expression '/' expression { $$ = makeBinary('/', $1, $3); }
        | expression '<' expression { $$ = makeBinary('<', $1, $3); }
        | expression '>' expression { $$ = makeBinary('>', $1, $3); }
        | expression EQ expression  { $$ = makeBinary(OP_EQ, $1, $3); }
        | expression NE expression  { $$ = makeBinary(OP_NE, $1, $3); }
        | expression LE expression  { $$ = makeBinary(OP_LE, $1, $3); }
        | expression GE expression  { $$ = makeBinary(OP_GE, $1, $3); }
        | expression AND expression { $$ = makeBinary(OP_AND, $1, $3); }
        | expression OR expression  { $$ = makeBinary(OP_OR, $1, $3); }
        | NOT expression            { $$ = newNode(N_NOT, 0, $2, NULL, NULL); }
        | '-' expression %prec UMINUS { $$ = newNode(N_NEG, 0, $2, NULL, NULL); }
        | '(' expression ')'        { $$ = $2; }
        | NUMBER                    { $$ = newNode(N_NUM, $1, NULL, NULL, NULL); }
        | ID
        {
            int i = valueIndex($1);
            $$ = (i >= 0) ? newNode(N_VAR, i, NULL, NULL, NULL) : newNode(N_NUM, 0, NULL, NULL, NULL);
            free($1);
        }
        ;

%%

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
