// These lines were automatically prepended
#ifndef SURFPACK_CONFIG_H
#define SURFPACK_CONFIG_H
#include "config.h"
#endif
// End of prepended lines
  
#include <iostream>
#include <map>
#include <set>
#include "PointDefinition.h"
#include "SurfpackParser.h"
#include "SurfpackInterpreter.h"
#include "Surface.h"
#include "SurfData.h"
#include "SurfPoint.h"
#include "SurfaceFactory.h"

using namespace std;

SurfpackInterpreter::SurfpackInterpreter() : parser(SurfpackParser::instance())
{

}

SurfpackInterpreter::~SurfpackInterpreter()
{

}

void SurfpackInterpreter::execute(std::istream& is, std::ostream& os, 
    std::ostream& es)
{
  if (parser.yyparse(&is, &os) == 0) {
    commandLoop(os, es);
  } else {
    es << "Parse error.  Command(s) not executed." << endl;
  }
}

void SurfpackInterpreter::commandLoop(std::ostream& os, std::ostream& es)
{
  const vector<SurfpackParser::ParsedCommand>& commands = parser.commandList();
  for (unsigned i = 0; i < commands.size(); i++) {
    try {
      if (commands[i].name == "LoadSurface") {
        os << commands[i].cmdstring << endl;
        executeLoadSurface(commands[i]);
      } else if (commands[i].name == "LoadData") {
        executeLoadData(commands[i]);
      } else if (commands[i].name == "SaveSurface") {
        executeSaveSurface(commands[i]);
      } else if (commands[i].name == "SaveData") {
        executeSaveData(commands[i]);
      } else if (commands[i].name == "ConvertData") {
        executeConvertData(commands[i]);
      } else if (commands[i].name == "ConvertSurface") {
        executeConvertSurface(commands[i]);
      } else if (commands[i].name == "CreateSurface") {
        executeCreateSurface(commands[i]);
      } else if (commands[i].name == "Evaluate") {
        executeEvaluate(commands[i]);
      } else if (commands[i].name == "Fitness") {
        executeFitness(commands[i]);
      } else if (commands[i].name == "PointDefinition") {
        executePointDefinition(commands[i]);
      } else if (commands[i].name == "GridPoints") {
        executeGridPoints(commands[i]);
      } else if (commands[i].name == "MonteCarloSample") {
        executeMonteCarloSample(commands[i]);
      } else {
        es << "Unrecognized command: " << commands[i].name << endl;
      }
    } catch (command_error& ce) {
      ce.print();
    }
  }
}
  
void SurfpackInterpreter::executeLoadData(
  const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this SurfData object
  string name = SurfpackParser::parseOutIdentifier(string("name"), command.arglist);
  if (name == "") {
    throw command_error(
      string("No name argument specified."), command.cmdstring);
  }
  //cout << "Variable name: " << name << endl;
  // Extract the name of the file to be read
  string filename = SurfpackParser::parseOutStringLiteral(string("file"), command.arglist);
  //cout << "Filename: " << filename << endl;
  if (filename == "") {
    throw command_error(
      string("No filename specified."), command.cmdstring);
  }
  // Read the file into a SurfData object and add it to the symbol table
  SurfData* sd = new SurfData(filename);
  symbol_table.dataVars.insert(SurfDataSymbol(name,sd));
  //cout << "Executed LoadData" << endl;
}

void SurfpackInterpreter::executeLoadSurface(
  const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this Surface object
  string name = SurfpackParser::parseOutIdentifier(string("name"), command.arglist);
  if (name == "") {
    throw command_error(
      string("No name argument specified."), command.cmdstring);
  }
  //cout << "Variable name: " << name << endl;
  // Extract the name of the file to be read
  string filename = SurfpackParser::parseOutStringLiteral(string("file"), command.arglist);
  //cout << "Filename: " << filename << endl;
  if (filename == "") {
    throw command_error(
      string("No filename specified."), command.cmdstring);
  }
  // Read the file into a Surface object and add it to the symbol table
  Surface* surf = SurfaceFactory::createSurface(filename);
  symbol_table.surfaceVars.insert(SurfaceSymbol(name,surf));
  
  //cout << "Executed LoadSurface" << endl;
}

void SurfpackInterpreter::executeSaveData(
  const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this SurfData object
  string data = SurfpackParser::parseOutIdentifier(string("data"), command.arglist);
  if (data == "") {
    throw command_error(
      string("No data argument specified."), command.cmdstring);
  }
  //cout << "Variable data: " << data << endl;
  // Extract the name of the file to be written to 
  string filename = SurfpackParser::parseOutStringLiteral(string("file"), command.arglist);
  //cout << "Filename: " << filename << endl;
  if (filename == "") {
    throw command_error(
      string("No filename specified."), command.cmdstring);
  }
  SurfDataMap::iterator iter = symbol_table.dataVars.find(data);
  if (iter == symbol_table.dataVars.end()) {
    //cout << "size of datavars: " << symbol_table.dataVars.size();
    SurfDataSymbol sd = *(symbol_table.dataVars.begin());
    //cout << "first: " << sd.first << endl;
    throw command_error(
      string("Symbol not found"), command.cmdstring);
  } else {
    SurfData* sd = iter->second;
    sd->write(filename);
  }
  //cout << "Executed SaveData" << endl;
}

void SurfpackInterpreter::executeConvertData(
  const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this SurfData object
  string inputfile = SurfpackParser::parseOutStringLiteral(string("input"), command.arglist);
  if (inputfile == "") {
    throw command_error(
      string("No input filename specified."), command.cmdstring);
  }
  //cout << "Input name: " << inputfile << endl;
  // Extract the name of the file to be read
  string outputfile = SurfpackParser::parseOutStringLiteral(string("output"), command.arglist);
  //cout << "Output name: " << outputfile << endl;
  if (outputfile == "") {
    throw command_error(
      string("No output filename specified."), command.cmdstring);
  }
  SurfData* sd = new SurfData(inputfile);
  sd->write(outputfile);
  //cout << "Converted " << inputfile << " to " << outputfile << endl;
}
void SurfpackInterpreter::executeConvertSurface(
  const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this SurfData object
  string inputfile = SurfpackParser::parseOutStringLiteral(string("input"), command.arglist);
  if (inputfile == "") {
    throw command_error(
      string("No input filename specified."), command.cmdstring);
  }
  //cout << "Input name: " << inputfile << endl;
  // Extract the name of the file to be read
  string outputfile = SurfpackParser::parseOutStringLiteral(string("output"), command.arglist);
  //cout << "Output name: " << outputfile << endl;
  if (outputfile == "") {
    throw command_error(
      string("No output filename specified."), command.cmdstring);
  }
  Surface* surf = SurfaceFactory::createSurface(inputfile);
  surf->write(outputfile);
  //cout << "Converted " << inputfile << " to " << outputfile << endl;
}
void SurfpackInterpreter::executeSaveSurface(
  const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this Surface object
  string surface = SurfpackParser::parseOutIdentifier(string("surface"), command.arglist);
  if (surface == "") {
    throw command_error(
      string("No surface argument specified."), command.cmdstring);
  }
  //cout << "Variable surface: " << surface << endl;
  // Extract the name of the file to be written to 
  string filename = SurfpackParser::parseOutStringLiteral(string("file"), command.arglist);
  //cout << "Filename: " << filename << endl;
  if (filename == "") {
    throw command_error(
      string("No filename specified."), command.cmdstring);
  }
  // Look up the surface in the symbol table 
  SurfaceMap::iterator iter = symbol_table.surfaceVars.find(surface);
  if (iter == symbol_table.surfaceVars.end()) {
    throw command_error(
      string("Symbol not found"), command.cmdstring);
  } else {
    Surface* surf = iter->second;
    surf->write(filename);
  }
  //cout << "Executed Load Surface" << endl;
}

void SurfpackInterpreter::executeCreateSurface(const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this SurfData object
  string name = SurfpackParser::parseOutIdentifier(string("name"), command.arglist);
  if (name == "") {
    throw command_error(
      string("No name argument specified."), command.cmdstring);
  }
  //cout << "Surface Variable name: " << name << endl;

  // Extract the type of surface 
  string type = SurfpackParser::parseOutStringLiteral(string("type"), command.arglist);
  if (type == "") {
    throw command_error(
      string("No surface type specified."), command.cmdstring);
  }
  //cout << "Surface type: " << type << endl;

  // Extract the name of the SurfData object (it should already be in the symbol table) 
  string dataName = SurfpackParser::parseOutIdentifier(string("data"), command.arglist);
  //cout << "Data name: " << dataName << endl;
  if (dataName == "") {
    throw command_error(
      string("No data object specified."), command.cmdstring);
  } else {
    SurfDataMap::iterator iter = symbol_table.dataVars.find(dataName);
    if (iter == symbol_table.dataVars.end()) {
      throw command_error(
        string("Data object not found"), command.cmdstring);
    }
    SurfData* sd = iter->second;
    Surface* surf = SurfaceFactory::createSurface(type, sd);
    surf->config(command.arglist);
    surf->createModel();
    symbol_table.surfaceVars.insert(SurfaceSymbol(name,surf));
  }
}

void SurfpackInterpreter::executeEvaluate(const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this SurfData object
  Surface* surf = 0;
  string surfaceName = SurfpackParser::parseOutIdentifier(string("surface"), command.arglist);
  if (surfaceName == "") {
    throw command_error(
      string("No existing surface specified."), command.cmdstring);
  } else {
    SurfaceMap::iterator iter = symbol_table.surfaceVars.find(surfaceName);
    if (iter == symbol_table.surfaceVars.end()) {
      throw command_error(
        string("Surface name not found."), command.cmdstring);
    } else {
      surf = iter->second;
    }
  }
  
  //cout << "Surface Variable name: " << surfaceName << endl;

  // Extract the name of the input data set (must be in the symbol table already) 
  string input_data = SurfpackParser::parseOutIdentifier(string("input_data"), command.arglist);
  SurfData* isd = 0;
  if (input_data == "") {
    throw command_error(
      string("No input_data specified."), command.cmdstring);
  } else {
    SurfDataMap::iterator iter = symbol_table.dataVars.find(input_data);
    if (iter == symbol_table.dataVars.end()) {
      throw command_error(
        string("Data object not found"), command.cmdstring);
    }
    isd = iter->second;
  }
  //cout << "input_data: " << input_data << endl;

  // Extract the name of the output data set 
  SurfData* osd = 0;
  string output_data = SurfpackParser::parseOutIdentifier(string("output_data"), command.arglist);
  if (output_data == "") {
    osd = isd;
  } else {
    //SurfDataMap::iterator iterForOutput = symbol_table.dataVars.find(dataName);
    //if (iterForOutput != symbol_table.dataVars.end()) {
    //  osd = iterForOutput->second;
    //} else {
      osd = new SurfData(*isd);
      symbol_table.dataVars.insert(SurfDataSymbol(output_data, osd));
    //}
    //cout << "output_data: " << output_data << endl;
  }

  // Now actually do the evaluation
  surf->getValue(*osd);
  
}

void SurfpackInterpreter::executeFitness(const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this Surface object
  Surface* surf = 0;
  string surfaceName = SurfpackParser::parseOutIdentifier(string("surface"), command.arglist);
  if (surfaceName == "") {
    throw command_error(
      string("No existing surface specified."), command.cmdstring);
  } else {
    SurfaceMap::iterator iter = symbol_table.surfaceVars.find(surfaceName);
    if (iter == symbol_table.surfaceVars.end()) {
      throw command_error(
        string("Surface name not found."), command.cmdstring);
    } else {
      surf = iter->second;
    }
  }
  
  //cout << "Surface Variable name: " << surfaceName << endl;

  // Extract the name of the SurfData object to be used (if any)
  string data = SurfpackParser::parseOutIdentifier(string("data"), command.arglist);
  SurfData* isd = 0;
  if (data != "") {
    SurfDataMap::iterator iter = symbol_table.dataVars.find(data);
    if (iter == symbol_table.dataVars.end()) {
      throw command_error(
        string("Data object not found"), command.cmdstring);
    }
    isd = iter->second;
  }
  //cout << "data: " << data << endl;

  // Extract the name of the output data set 
  string metric = SurfpackParser::parseOutStringLiteral(string("metric"), command.arglist);
  if (metric == "") {
      throw command_error(
        string("No fitness metric specified."), command.cmdstring);
  }
  double fitness = surf->goodnessOfFit(metric, isd);
  cout << metric << " fitness value for surface " << surfaceName << ": "
       << fitness << endl;
  
}
  
void SurfpackInterpreter::executePointDefinition(const SurfpackParser::ParsedCommand& command)
{

  // Extract the variable name for this SurfData object
  string name = SurfpackParser::parseOutIdentifier(string("name"), command.arglist);
  if (name == "") {
    throw command_error(
      string("No name argument specified."), command.cmdstring);
  }
  //cout << "Variable name: " << name << endl;

  // Extract the name of the file to be read
  string filename = SurfpackParser::parseOutStringLiteral(string("file"), command.arglist);
  //cout << "Filename: " << filename << endl;
  if (filename == "") {
    throw command_error(
      string("No filename specified."), command.cmdstring);
  }

  // Read the file into a SurfData object and add it to the symbol table
  PointDefinition* sd = new PointDefinition(filename);
  symbol_table.pointDefinitionVars.insert(PointDefinitionSymbol(name,sd));
  //cout << "Executed PointDefinition" << endl;
}

void SurfpackInterpreter::executeGridPoints(const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this SurfData object
  string def = SurfpackParser::parseOutIdentifier(string("def"), command.arglist);
  if (def == "") {
    throw command_error(
      string("No def argument specified."), command.cmdstring);
  }
  PointDefinitionMap::iterator iter = symbol_table.pointDefinitionVars.find(def);
  if (iter == symbol_table.pointDefinitionVars.end()) {
    throw command_error(
      string("Definition not found in symbol table."), command.cmdstring);
  }
  //cout << "Variable def: " << def << endl;
  PointDefinition* pd = iter->second;

  // Extract the name of the SurfData object (it may already be in the symbol table) 
  string dataName = SurfpackParser::parseOutIdentifier(string("data"), command.arglist);
  //cout << "Data name: " << dataName << endl;
  if (dataName == "") {
    throw command_error(
      string("No data object specified."), command.cmdstring);
  } 
  SurfDataMap::iterator sditer = symbol_table.dataVars.find(dataName);
  if (sditer != symbol_table.dataVars.end()) {
  // Delete the one that's there because we're going to replace it
    delete sditer->second;
    symbol_table.dataVars.erase(sditer);
  } 
  vector<string> testFunctions;
  const SurfpackParser::ArgList& args = command.arglist;
  for (unsigned i = 0; i < args.size(); i++) {
    if (args[i].name == "test_function") {
      testFunctions.push_back(args[i].lval.literal);
    }
  }
  SurfData* gridData = pd->sampleGrid(testFunctions);
  symbol_table.dataVars.insert(SurfDataSymbol(dataName, gridData));
  //cout << "Executed GridPoints" << endl;
}

void SurfpackInterpreter::executeMonteCarloSample(const SurfpackParser::ParsedCommand& command)
{
  // Extract the variable name for this SurfData object
  string def = SurfpackParser::parseOutIdentifier(string("def"), command.arglist);
  if (def == "") {
    throw command_error(
      string("No def argument specified."), command.cmdstring);
  }
  PointDefinitionMap::iterator iter = symbol_table.pointDefinitionVars.find(def);
  if (iter == symbol_table.pointDefinitionVars.end()) {
    throw command_error(
      string("Definition not found in symbol table."), command.cmdstring);
  }
  //cout << "Variable def: " << def << endl;
  PointDefinition* pd = iter->second;

  // Extract the name of the SurfData object (it may already be in the symbol table) 
  string name = SurfpackParser::parseOutIdentifier(string("name"), command.arglist);
  //cout << "Data name: " << name << endl;
  if (name == "") {
    throw command_error(
      string("No name for resulting data object specified."), command.cmdstring);
  } 
  SurfDataMap::iterator sditer = symbol_table.dataVars.find(name);
  if (sditer != symbol_table.dataVars.end()) {
  // Delete the one that's there because we're going to replace it
    delete sditer->second;
    symbol_table.dataVars.erase(sditer);
  } 
  vector<string> testFunctions;
  unsigned numSamples = 100; // default value
  const SurfpackParser::ArgList& args = command.arglist;
  for (unsigned i = 0; i < args.size(); i++) {
    if (args[i].name == "test_function") {
      testFunctions.push_back(args[i].lval.literal);
    } else if (args[i].name == "size") {
      numSamples = args[i].lval.integer;
    }
  }
  SurfData* gridData = pd->sampleMonteCarlo(numSamples, testFunctions);
  symbol_table.dataVars.insert(SurfDataSymbol(name, gridData));
  //cout << "Executed MonteCarloSample" << endl;

}
