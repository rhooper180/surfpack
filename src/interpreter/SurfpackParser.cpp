#include "surfpack_config.h"
#include "SurfpackParser.h"
#include "surfparse.h"
#include "FlexWrapper.h"
extern int yyparse();


std::ostringstream SurfpackParser::cmdstream;

SurfpackParser& SurfpackParser::instance()
{
  static SurfpackParser sp;
  return sp;
}

FlexWrapper& SurfpackParser::globalLexer()
{
  return *global_lexer;
}

int SurfpackParser::yyparse(std::istream& is, std::ostream& os)
{
  global_lexer->setParseStreams(is,os);
  return ::yyparse();
}

SurfpackParser::SurfpackParser() 
{
  global_lexer = new FlexWrapper;
  currentTuple = new Tuple;
  init();
}

void SurfpackParser::init()
{
  commands.clear();
  currentArgList = 0;
  currentArgIndex = -1;
  currentTupleIndex = -1;
  cmdstream.str("");
}

SurfpackParser::~SurfpackParser()
{
  delete global_lexer;
  delete currentTuple;
  global_lexer = 0;
}

void SurfpackParser::storeCommandString()
{
  if (commands.size() > 0) {
    int loc;
    std::string newcommand = cmdstream.str();
    if (newcommand.find("/*") == 0) {
      newcommand.erase(0,2);
    }
    if ((loc = newcommand.find("*/")) != std::string::npos) {
      newcommand.erase(loc,2);
    }
    if (newcommand.find("!") == 0) {
       newcommand.erase(0,1);
    }
    commands[commands.size()-1].cmdstring = newcommand;
    cmdstream.str("");
  }
}

std::vector<ParsedCommand>& SurfpackParser::commandList()
{
  return commands;
}

void SurfpackParser::print()
{
  //for (unsigned i = 0; i < commands.size(); i++) {
  //  std::cout << commands[i].name << std::endl;
  //  for (unsigned j = 0; j < commands[i].arglist.size(); j++) {
  //    std::cout << "   " << commands[i].arglist[j].name << " ";
  //    if (commands[i].arglist[j].rval.tuple.size() != 0) {
  //      for (unsigned k = 0; k < commands[i].arglist[j].rval.tuple.size(); k++) {
  //        std::cout << commands[i].arglist[j].rval.tuple[k] << " ";
  //      }
  //    } else if (commands[i].arglist[j].rval.triplet.numPts != 0) {
  //      std::cout << "{" 
  //           <<	commands[i].arglist[j].rval.triplet.min
  //           << ","
  //           <<	commands[i].arglist[j].rval.triplet.max
  //           << ","
  //           <<	commands[i].arglist[j].rval.triplet.numPts
  //           << "}" ;
  //    } 
  //    std::cout << std::endl;
  //  }
  //}
}

void SurfpackParser::addCommandName()
{
  //std::cout << "Add command name" << std::endl;
  ParsedCommand pc;
  commands.push_back(pc);
  currentArgList = &(commands[commands.size()-1].arglist);
  currentArgIndex = -1;
  commands[commands.size()-1].name = std::string(global_lexer->currentToken());
}

void SurfpackParser::addArgName()
{
  //std::cout << "Add arg name" << std::endl;
  if (currentArgList == 0) {
    std::cerr << "currentArgList is NULL; cannot assign name" << std::endl;
  } else {
    Arg newArg;
    currentArgList->push_back(newArg);
    currentArgIndex++;
    (*currentArgList)[currentArgIndex].name = std::string(global_lexer->currentToken());
  }
}

void SurfpackParser::addArgValIdent()
{
  if (currentArgIndex == -1) {
    std::cerr << "currentArgIndex = -1; cannot assign Identifier" << std::endl;
  } else {
    (*currentArgList)[currentArgIndex].setRVal
      (new RvalIdentifier(std::string(global_lexer->currentToken())));
  }
}

void SurfpackParser::addArgValInt()
{
  if (currentArgIndex == -1) {
    std::cerr << "currentArgIndex = -1; cannot assign Integer" << std::endl;
  } else {
    (*currentArgList)
      [currentArgIndex].setRVal(new RvalInteger(atoi(global_lexer->currentToken())));
  }
}

void SurfpackParser::addArgValString()
{
  if (currentArgIndex == -1) {
    std::cerr << "currentArgIndex = -1; cannot assign String" << std::endl;
  } else {
    std::string currentToken = std::string(global_lexer->currentToken());
    // The token contains leading and trailing apostrophes; remove them.
    int pos;
    while ( (pos = currentToken.find('\'')) != std::string::npos) {
      currentToken.erase(pos,pos+1);
    }
    (*currentArgList)
      [currentArgIndex].setRVal(new RvalStringLiteral(currentToken));
    //std::cout << "Stripped std::string: " << currentToken << std::endl;
  }
}

void SurfpackParser::addArgValReal()
{
  if (currentArgIndex == -1) {
    std::cerr << "currentArgIndex = -1; cannot assign Real" << std::endl;
  } else {
    (*currentArgList)
      [currentArgIndex].setRVal(new RvalReal(atof(global_lexer->currentToken())));
  }
}

void SurfpackParser::addArgValTuple()
{
  if (currentArgIndex == -1) {
    std::cerr << "currentArgIndex = -1; cannot assign Tuple" << std::endl;
  } else {
    currentTupleIndex = -1;
  }
}

void SurfpackParser::addArgValArgList()
{
  //if (currentArgIndex == -1) {
  //  std::cerr << "currentArgIndex = -1; cannot assign ArgList" << std::endl;
  //} else {
  //  currentArgList = &((*currentArgList)[currentArgIndex].rval.arglist);
  //}
}

void SurfpackParser::addNumberAsTriplet()
{
  //if (currentArgIndex == -1) {
  //  std::cerr << "currentArgIndex = -1; cannot addNumberAsTriplet" << std::endl;
  //} else {
  //  double val = atof(global_lexer->currentToken());
  //  (*currentArgList)[currentArgIndex].rval.triplet.min = val;
  //  (*currentArgList)[currentArgIndex].rval.triplet.max = val;
  //  (*currentArgList)[currentArgIndex].rval.triplet.numPts= 1;
  //}
}

void SurfpackParser::addTriplet()
{

}

void SurfpackParser::addTripletMin()
{
  //if (currentArgIndex == -1) {
  //  std::cerr << "currentArgIndex = -1; cannot addTripletMin" << std::endl;
  //} else {
  //  double val = atof(global_lexer->currentToken());
  //  (*currentArgList)[currentArgIndex].rval.triplet.min = val;
  //}
}

void SurfpackParser::addTripletMax()
{
  //if (currentArgIndex == -1) {
  //  std::cerr << "currentArgIndex = -1; cannot addTripletMax" << std::endl;
  //} else {
  //  double val = atof(global_lexer->currentToken());
  //  (*currentArgList)[currentArgIndex].rval.triplet.max = val;
  //}
}

void SurfpackParser::addTripletNumPts()
{
  //if (currentArgIndex == -1) {
  //  std::cerr << "currentArgIndex = -1; cannot addTripletMax" << std::endl;
  //} else {
  //  int val = atoi(global_lexer->currentToken());
  //  (*currentArgList)[currentArgIndex].rval.triplet.numPts = val;
  //}
}

void SurfpackParser::addTupleVal()
{
  if (currentArgIndex == -1) {
    std::cerr << "currentArgIndex = -1; cannot addTupleVal" << std::endl;
  } else {
    double val = atof(global_lexer->currentToken());
    currentTuple->push_back(val);
    //(*currentArgList)[currentArgIndex].getRVal()->getTuple().push_back(val);
  }
}

void SurfpackParser::addTuple()
{
  if (currentArgIndex == -1) {
    std::cerr << "currentArgIndex = -1; cannot addTuple" << std::endl;
  } else {
    (*currentArgList)[currentArgIndex].setRVal(new RvalTuple(*currentTuple));
  }
}

void SurfpackParser::newTuple()
{
  currentTuple->clear();
}

std::string SurfpackParser::parseOutIdentifier(const std::string& argname,
  const ArgList& arglist)
{
  for (unsigned i = 0; i < arglist.size(); i++) {
    if (arglist[i].name == argname) {
      return arglist[i].getRVal()->getIdentifier();
    }
  }
  return std::string("");
}

std::string SurfpackParser::parseOutStringLiteral(const std::string& argname,
  const ArgList& arglist)
{
  for (unsigned i = 0; i < arglist.size(); i++) {
    if (arglist[i].name == argname) {
      return arglist[i].getRVal()->getStringLiteral();
    }
  }
  return std::string("");
}

int SurfpackParser::parseOutInteger(const std::string& argname,
  const ArgList& arglist, bool& valid)
{
  valid = false;
  for (unsigned i = 0; i < arglist.size(); i++) {
    if (arglist[i].name == argname) {
      valid = true;
      return arglist[i].getRVal()->getInteger();
    }
  }
  return -1;
}

void SurfpackParser::shellCommand()
{
  ParsedCommand pc(true); // ctor arg specifies that it's a shell command
  commands.push_back(pc);
  storeCommandString();
}

ParsedCommand::ParsedCommand() : shellCommand(false)
{

}

ParsedCommand::ParsedCommand(bool shell_command) 
  : shellCommand(shell_command)
{

}

bool ParsedCommand::isShellCommand() const 
{ 
  return shellCommand; 
}

int yylex()
{
  return SurfpackParser::instance().globalLexer().nextToken();
}

void appendToken(const char* token)
{
  SurfpackParser::cmdstream << token;
}
