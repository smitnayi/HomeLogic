HomeLogic - Smart Home Automation Language (LEX + YACC)

Build:
  bison -d HomeLogic.y
  flex HomeLogic.l
  gcc HomeLogic.tab.c lex.yy.c -o homelogic

Run:
  homelogic program.home      (valid program, asks for a temperature reading)
  homelogic error.home        (shows lexical, syntax and semantic errors)
