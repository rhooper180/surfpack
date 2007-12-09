/*  _______________________________________________________________________
 
    Surfpack: A Software Library of Multidimensional Surface Fitting Methods
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
    For more information, see the README file in the top Surfpack directory.
    _______________________________________________________________________ */

#ifdef HAVE_CONFIG_H
#include "surfpack_config.h"
#endif
#include "surfpack_system_headers.h"
#include "AxesBounds.h"
#include "SurfpackParser.h"
class SurfData;
class Surface;
class ParsedCommand;
class SurfpackParser;
class SurfpackModel;

class SurfpackInterpreter
{
public:
  SurfpackInterpreter();
  ~SurfpackInterpreter();
  void execute(const std::string* input_string = 0, 
    const std::string* output_string = 0);
  void commandLoop(std::ostream& os = std::cout, std::ostream& es = std::cerr);
  void commandLoopOld(std::ostream& os = std::cout, std::ostream& es = std::cerr);

  // individual surfpack commands
  void executeCreateAxes(const ParsedCommand& command);
  void executeCreateSample(const ParsedCommand& command);
  void executeCreateSurface(const ParsedCommand& command);
  void executeEvaluate(const ParsedCommand& command);
  void executeFitness(const ParsedCommand& command);
  void executeLoad(const ParsedCommand& command);
  void executeLoadData(const ParsedCommand& command);
  void executeLoadSurface(const ParsedCommand& command);
  void executeSave(const ParsedCommand& command);
  void executeSaveData(const ParsedCommand& command);
  void executeSaveSurface(const ParsedCommand& command);
  void executeShellCommand(const ParsedCommand& command);

  void execCreateAxes(ParamMap& args);
  void execCreateSample(ParamMap& args);
  void execCreateSurface(ParamMap& args);
  void execEvaluate(ParamMap& args);
  void execFitness(ParamMap& args);
  void execLoad(ParamMap& args);
  void execLoadData(ParamMap& args);
  void execLoadSurface(ParamMap& args);
  void execSave(ParamMap& args);
  void execSaveData(ParamMap& args);
  void execSaveSurface(ParamMap& args);
  void execShellCommand(ParamMap& args);

  ///\todo Collapse all of these conversion functions into one template function
  static int asInt(const std::string& arg);
  static std::string asStr(const std::string& arg);
  static double asDbl(const std::string& arg);
  static VecUns asVecUns(const std::string& arg);
  static VecStr asVecStr(const std::string& arg);
  static int asInt(const std::string& arg, bool& valid);
  static std::string asStr(const std::string& arg, bool& valid);
  static double asDbl(const std::string& arg, bool& valid);
  static VecUns asVecUns(const std::string& arg, bool& valid);
  static VecStr asVecStr(const std::string& arg, bool& valid);
protected:
  class command_error 
  {
  public:
    command_error(const std::string& msg_in = "", 
      const std::string& cmdstring_in = "") 
      : msg(msg_in), cmdstring(cmdstring_in) {}
    void print() { std::cerr << "Error in " << cmdstring << ":  " 
			     << msg << std::endl; }
  protected:
    std::string msg;
    std::string cmdstring;
  };
public:
  typedef std::pair<std::string, SurfData*> SurfDataSymbol;
  typedef std::map<std::string, SurfData*> SurfDataMap;
  typedef std::pair<std::string, Surface*> SurfaceSymbol;
  typedef std::map<std::string, Surface*> SurfaceMap;
  typedef std::pair<std::string, AxesBounds*> AxesBoundsSymbol;
  typedef std::map<std::string, AxesBounds*> AxesBoundsMap;
  typedef std::pair<std::string, SurfpackModel*> SurfpackModelSymbol;
  typedef std::map<std::string, SurfpackModel*> SurfpackModelMap;
  
private:
  struct SymbolTable
  {
    SurfDataMap dataVars;
    SurfaceMap surfaceVars;
    SurfpackModelMap modelVars;
    AxesBoundsMap axesVars;
    ~SymbolTable();  
    SurfpackModel* lookupModel(const std::string);
    Surface* lookupSurface(const std::string);
    SurfData* lookupData(const std::string);
    AxesBounds* lookupAxes(const std::string);
  };

  struct SymbolTable symbolTable;
  SurfpackParser& parser;
};
    
