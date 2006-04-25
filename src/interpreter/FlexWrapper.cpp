/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2001, Sandia National Laboratories.

    Surfpack: A Software Library of Multidimensional Surface Fitting Methods

    Surfpack is distributed under the DAKOTA GNU General Public License.
    For more information, see the README file in the top Surfpack directory.
    _______________________________________________________________________ */
#include "FlexWrapper.h"

// Symbols and functions that will be implemented through flex

/// The Current token
extern char* yytext;

/// The input stream for the lexer
extern FILE* yyin;

/// The output stream for the lexer
extern FILE* yyout;

/// The "next token" method-- gets called by bison-generated code
extern "C" int yylex();

FlexWrapper::FlexWrapper()
  : infile(0), outfile(0)
{
}

FlexWrapper::~FlexWrapper()
{
  if (infile) fclose(infile);
  if (outfile) fclose(outfile);
}

void FlexWrapper::setParseStreams(const std::string* input_string, 
  const std::string* output_string)
{
  if (input_string) {
    infile = fopen(input_string->c_str(),"r");
  }
  if (output_string) {
    outfile = fopen(output_string->c_str(),"w");
  }
  yyin = infile;
  yyout = outfile;
}

int FlexWrapper::nextToken()
{
  return yylex();
}

const char* FlexWrapper::currentToken()
{
  return yytext;
}
