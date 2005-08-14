#ifndef MY_PARSING_STRUCTURES_H
#define MY_PARSING_STRUCTURES_H
#include "surfpack_config.h"
#include "surfpack_system_headers.h"
#include "SurfpackParserArgs.h"

class FlexWrapper;

class ParsedCommand
{ 
public:
  ParsedCommand();
  ParsedCommand(bool shell_command);
  bool isShellCommand() const;
  bool shellCommand;
  std::string name;
  ArgList arglist;
  std::string cmdstring;
};

class SurfpackParser 
{
public:
  // commands for use by the Bison parser
  void addCommandName();
  void addArgName();
  void addArgValIdent();
  void addArgValInt();
  void addArgValString();
  void addArgValReal();
  void addArgValTuple();
  void addArgValArgList();
  void addArgListToCommand();
  void addNumberAsTriplet();
  void addTriplet();
  void addTripletMin();
  void addTripletMax();
  void addTripletNumPts();
  void addTupleVal();
  void newTuple();
  void pushNewArgList();
  void popArgList();
  void init();
  void print();
  void storeCommandString();
  void shellCommand();


  static SurfpackParser& instance();
  static std::ostringstream cmdstream;
  FlexWrapper& globalLexer();
  int yyparse(std::istream& is = std::cin, std::ostream& os = std::cout);
  std::vector<ParsedCommand>& commandList();

  // commands for use by interpreter to parse out certain argument types
  static std::string parseOutIdentifier(const std::string& argname,
    const ArgList& arglist);
  static std::string parseOutStringLiteral(const std::string& argname,
    const ArgList& arglist);
  static int parseOutInteger(const std::string& argname,
    const ArgList& arglist, bool& valid);
protected:
// Default constructor, copy constructor and assignment operator declared 
// protected and not implemented, in order to make sure that only one
// instance is created
  SurfpackParser();
  ~SurfpackParser();
  SurfpackParser(const SurfpackParser&);
  const SurfpackParser& operator=(const SurfpackParser&);

private:
  std::vector<ParsedCommand> commands;
  ArgList* currentArgList;
  int currentArgIndex;
  int currentTupleIndex;
  FlexWrapper* global_lexer;
  Tuple* currentTuple;
  std::stack< ArgList > arglistStack;
};

#endif




