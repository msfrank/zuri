lexer grammar RequirementLexer;

//

WholeNumber                 : ('0' | '1'..'9' '0'..'9'*) ;

// reserved sigils

DotOperator                 : '.' ;
TildeOperator               : '~' ;
CaretOperator               : '^' ;
RangeOperator               : '-' ;
StarOperator                : '*' ;
CommaOperator               : ',' ;


/** other definitions */

EXPRWS                      : [ \t\r\n]+ -> skip ;  // skip whitespace