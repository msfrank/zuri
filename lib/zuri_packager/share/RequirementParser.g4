parser grammar RequirementParser;

options { tokenVocab = RequirementLexer; }


requirementsList            : requirement ( CommaOperator requirement )* ;

requirement                 : exactVersionOrHyphenRange
                            | starRangeRequirement
                            | tildeRangeRequirement
                            | caretRangeRequirement
                            ;

fullVersion                 : WholeNumber DotOperator WholeNumber DotOperator WholeNumber ;
apiVersion                  : WholeNumber DotOperator WholeNumber ;
majorVersion                : WholeNumber ;
version                     : fullVersion | apiVersion | majorVersion ;

starRangeRequirement        : WholeNumber DotOperator WholeNumber DotOperator StarOperator
                            | WholeNumber DotOperator StarOperator
                            | StarOperator
                            ;

tildeRangeRequirement       : TildeOperator version ;

caretRangeRequirement       : CaretOperator version ;

exactVersionOrHyphenRange   : version RangeOperator version         # hyphenRangeRequirement
                            | version                               # exactVersionRequirement
                            ;
