#ifndef TYPES_H
#define TYPES_H

typedef enum stype {
    S_NONE, S_INT, S_BOOL, S_STRING, S_UNIT, S_LABEL, S_ARRAY, S_FUNCTION, S_PROG
} stype;

typedef enum {
  Q_PLUS, Q_MINUS, Q_MULT, Q_DIV, Q_EXP, Q_INF, Q_INFEQ, Q_SUP, Q_SUPEQ, Q_EQUAL, Q_DIFF, Q_AND, Q_OR, Q_XOR, Q_NOT, Q_MOD,// operators (binary or unary)
  Q_FUNDEC, Q_FUNEND, Q_FUNCALL, Q_FUNRETURN, // for functions
  Q_END, Q_WRITE, Q_READ, Q_AFFEC, Q_LABEL, Q_GOTO, Q_IF, Q_IFELSE, Q_MAIN // other
} qop;
#endif
