// These lines were automatically prepended
#ifndef SURFPACK_CONFIG_H
#define SURFPACK_CONFIG_H
#include "config.h"
#endif
// End of prepended lines

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <set>
#include <cmath>
#include <string>
#include <ctime>
#include <cstdlib>

#include "SurfPoint.h"
#include "SurfData.h"
#include "Surface.h"
//#include "SurfDataIterator.h"
#include "PolynomialSurface.h"
#include "KrigingSurface.h"
#include "MarsSurface.h"
#include "surfpack.h"
#include "ANNSurface.h"
//#include "ParticleSwarmMLP.h"
//#include "BackpropMLP.h"


typedef struct {
    double min;
    double max;
    int pts;
    double interval;
} Axis;


using namespace std;

vector<Axis> axes;
vector<int> point;
vector<double> surfptx;
int ndims;
int npts;


void readInput(istream& is = cin)
{
    npts = 1;
    const int MAX_CHAR = 200;
    char line[MAX_CHAR];
    is.getline(line,MAX_CHAR);
    string sline = line;
    istringstream istream(sline);
    istream >> ndims;
    axes.resize(ndims);
    for (int i = 0; i < ndims; i++) {
        is.getline(line,MAX_CHAR);
        sline = line;
	istringstream streamline(sline);
	string thestring = streamline.str();
        char c;
        streamline >> c;
	if (c == 'v') {
	    streamline >> axes[i].min >> axes[i].max >> axes[i].pts;
            axes[i].interval = (axes[i].max - axes[i].min) / (axes[i].pts - 1);
	    npts *= axes[i].pts;
        } else if (c == 'f') {
            streamline >> axes[i].min;
	    axes[i].max = axes[i].min;
	    axes[i].pts = 1;
	    axes[i].interval = 0.0;
	} else {
    	    cerr << "Expected 'f' or 'v'" << endl;
	}
    }
}    
     
void initialize()
{
    point.resize(ndims);
    for (int i = 0; i < ndims; i++) {
	point[i] = 0;
    }
}

void nextPoint()
{
    int curDim = ndims-1;
    while (axes[curDim].pts == 1 || point[curDim] == (axes[curDim].pts - 1)) {
	curDim--;
    }
    if (curDim < ndims) {
        point[curDim]++;
	curDim++;
	while(curDim < ndims) {
	    point[curDim] = 0;
	    curDim++;
	}
    }
}
void populateSurfData(SurfData* surfData, string filename, Surface* surface=0)
{
    ifstream infile(filename.c_str(),ios::in);
    if (!infile) {
	    cerr << "File " << filename << " not found." << endl;
	    return;
    }
    readInput(infile);
    infile.close();
    initialize();
    surfptx.resize(ndims);
    for (int i = 0; i < npts; i++) {
        for (int j = 0; j < ndims; j++) {
	    surfptx[j] = (axes[j].max - axes[j].min)*((double)rand()/(double)INT_MAX)+ axes[j].min;
    	    //cout << setw(10) << surfptx[j];
	}
	//cout << endl;
	//double fx = surface ? surface->evaluate(surfptx) : rastrigin(surfptx)*.5;
        SurfPoint sp(surfptx);
	surfData->addPoint(sp);
	nextPoint();
    }
    //cout << "Number of points: " << npts << endl;
}

SurfData pointSpecToSurfData(string filename) {
  ifstream infile(filename.c_str(),ios::in);
  if (!infile) {
          cerr << "File " << filename << " not found." << endl;
	  exit(1);
  }
  readInput(infile);
  infile.close();
  initialize();
  surfptx.resize(ndims);
  vector<SurfPoint> sps;
  for (int i = 0; i < npts; i++) {
      for (int j = 0; j < ndims; j++) {
          surfptx[j] = axes[j].min + axes[j].interval*point[j];
  	    //cout << setw(10) << surfptx[j];
      }
      //cout << endl;
      //double fx = surface ? surface->evaluate(surfptx) : rastrigin(surfptx)*.5;
      SurfPoint sp(surfptx);
      sps.push_back(sp);
      //surfData->addPoint(sp);
      nextPoint();
  }
  return SurfData(sps);
}

SurfData randomPoints(string filename) {
  ifstream infile(filename.c_str(),ios::in);
  if (!infile) {
          cerr << "File " << filename << " not found." << endl;
          exit(1);
  }
  readInput(infile);
  infile.close();
  initialize();
  surfptx.resize(ndims);
  vector<SurfPoint> sps;
  for (int i = 0; i < npts; i++) {
      for (int j = 0; j < ndims; j++) {
	surfptx[j] = (axes[j].max - axes[j].min)*((double)rand()/(double)INT_MAX)+ axes[j].min;
  	    //cout << setw(10) << surfptx[j];
      }
      //cout << endl;
      //double fx = surface ? surface->evaluate(surfptx) : rastrigin(surfptx)*.5;
      SurfPoint sp(surfptx);
      sps.push_back(sp);
      //surfData->addPoint(sp);
      nextPoint();
  }
  return SurfData(sps);
}

double sphere(const vector<double>& pt) 
{
  double result = 0.0;
  for (unsigned i = 0; i < pt.size(); i++) {
    double x = pt[i];
    result += x*x;
  }
  return result;
}

double rastrigin(const vector<double>& pt) 
{
  double result = 0.0;
  for (unsigned i = 0; i < pt.size(); i++) {
    double x = pt[i];
    result += x*x-10*cos(4.0*acos(0.0)*x)+10.0;
  }
  return result;
}

double rosenbrock(const vector<double>& pt) 
{
  double result = 0.0;
  for (unsigned i = 0; i < pt.size() - 1; i++) {
    double x = pt[i];
    double xp = pt[i+1];
    result += 100.0*(xp-x*x)*(xp-x*x)+(x-1.0)*(x-1.0); 
  }
  return result;
}


double testFunction(const string name, const vector<double>& pt)
{
  if (name == "rosenbrock") {
    return rosenbrock(pt);
  } else if (name == "sphere") {
    return sphere(pt);
  } else {
    return rastrigin(pt);
  }
}

void randomSample(vector< string >& args) 
{
  SurfData sd = randomPoints(args[1]);
  if (args.size() == 4) {
    vector<double> newResponseValues(sd.size());
    //unsigned newindex = sd->addResponse();
    for (unsigned i = 0; i < sd.size(); i++) {
      newResponseValues[i] = testFunction(args[3],sd[i].X());
      //sd->Point(i).F(newindex,response);
    }
    sd.addResponse(newResponseValues);
  }
  sd.write(args[2]);
}

void gridPoints(vector< string >& args) 
{
  SurfData sd = pointSpecToSurfData(args[1]);
  if (args.size() == 4) {
    vector<double> newResponseValues(sd.size());
    //unsigned newindex = sd->addResponse();
    for (unsigned i = 0; i < sd.size(); i++) {
      newResponseValues[i] = testFunction(args[3],sd[i].X());
      //double response = testFunction(args[3],sd->Point(i).X());
      //sd->Point(i).F(newindex,response);
    }
    sd.addResponse(newResponseValues);
  }
  sd.write(args[2]);
}

void create(vector< string >& args)
{
    Surface* s = 0;
    SurfData sd(args[1]);
    //ifstream infile(args[1].c_str(), ios::in);
    //if (!infile) {
    //        cerr << "Error: unable to open" << args[1] << "." << endl;
    //        return;
    //}
    //sd.read(infile);
    
    //ofstream outfile(args[2].c_str(), ios::out);
    //if (!outfile) {
    //        cerr << "Error: unable to open " << args[2] << " for output." << endl;
    //        return;
    //}
    if (args[3] == "Polynomial" && args.size() == 5) {
      // must be a polynomial surface 
      sd.setDefaultIndex(0);
      s = createSurface(args[3], sd, atoi(args[4].c_str()));
    } else {
      // create surface with sd; responseIndex = 0
      sd.setDefaultIndex(0);
      s = createSurface(args[3], sd);
    }
    if (args[3] == "Kriging") {
      if (args.size() > 5) {
        vector<double> vals;
        for (unsigned argInd = 5; argInd < args.size(); argInd++) {
          vals.push_back(atof(args[argInd].c_str()));
        }
        if (args[4] == "ConminSeed") {
 	  cout << "Setting conmin seed" << endl;
          (dynamic_cast<KrigingSurface*>(s))->setConminThetaVars(vals);
        } else if (args[4] == "Thetas") {
 	  cout << "Setting theta vars" << endl;
          (dynamic_cast<KrigingSurface*>(s))->usePreComputedCorrelationVector(vals);
        }
      }
    }
        
     
    //if (args[3] == "kriging") {
    //        s = new KrigingSurface(&sd);
    //        s->build();
    //        s->test(&sd);
    //} else if (args[3] == "polynomial") {
    //        int order = atoi(args[4].c_str());
    //        s = new PolynomialSurface(&sd,order);
    //        s->build();
    //}
    if (s) {
            s->createModel();
	    s->write(args[2]);
    }
    //infile.close();
    delete s;
}

void evaluateSurface(vector< string >& args)
{
    Surface* s = 0;
    SurfData sd(args[1]);
    //ifstream infile(args[1].c_str(), ios::in);
    //if (!infile) {
    //        cerr << "Error: unable to open" << args[1] << " for input." << endl;
    //        return;
    //}
    //sd.read(infile);

    //ifstream insurface(args[3].c_str(), ios::in | ios::binary);
    //if (!insurface) {
    //        cerr << "Error: unable to open" << args[3] << " for input." << endl;
    //        return;
    //}
    //
    ofstream outfile(args[2].c_str(), ios::out);
    if (!outfile) {
            cerr << "Error: unable to open " << args[2] << " for output." << endl;
            return;
    }
    //int surfaceID;
    //insurface.read((char*)&surfaceID, sizeof(int));
    //insurface.close();
    //switch (surfaceID) {
    //        case Surface::polynomialSurfaceID: 
    //            s=new PolynomialSurface(args[3]);
    //    	cout << "Polynomial" << endl;
    //            break;
    //        case Surface::krigingSurfaceID: 
    //    	s = new KrigingSurface(args[3]);
    //            cout << "Kriging" << endl; 
    //    	break;
    //        default:
    //    	cout << "Unknown Surface" << endl;
    //}
    s = createSurface(args[3]);
    s->getValue(sd);
    
    sd.writeText(outfile);
    //infile.close();
    //outfile.close();
    delete s;
}

void computeErrorMetric(vector< string>& args)
{
    Surface* s = createSurface(args[1]);
    SurfData* sd = 0;
    //AbstractSurfDataIterator* itr = 0;
    if (args.size() == 4) {
      sd = new SurfData(args[3]);
      //itr = new SurfDataIterator(*sd);
    }
    if (s) {
      double errorValue = s->goodnessOfFit(args[2], sd);
      cout << args[2] << ": " << errorValue << endl;
      delete s;
    }
    //delete itr;
    delete sd;


}

void conversion(vector< string>& args)
{
  if (args[1].find(".txt") == args[1].size() - 4 || 
      args[2].find(".txt") == args[2].size() - 4) {
    if (args[1].find(".sd") == args[1].size() - 3 || 
        args[2].find(".sd") == args[2].size() - 3) {
      cout << "Converting SurfData..." << endl;
      SurfData sd(args[1]);
      sd.write(args[2]);
    } else if (args[1].find(".srf") == args[1].size() - 4 || 
        args[2].find(".srf") == args[2].size() - 4) {
      cout << "Converting Surface..." << endl;
      Surface* s = createSurface(args[1]);
      s->write(args[2]);
      delete s;
    } else {
      cerr << "One of the files must be .txt.  "
           << "The other must be .sd (SurfData) or "
           << " .srf (Surface)." << endl;
    }
  } else {
    cerr << "One of the files must be .txt.  "
         << "The other must be .sd (SurfData) or "
         << " .srf (Surface)." << endl;
  }
}

void executeCommand(vector< string >& args) 
{
	cout << "Executing command: ";
	for (unsigned i = 0; i < args.size(); i++) {
		cout << args[i] << " ";
	}
	cout << endl;

	if (args[0] == "gridpoints") {
		gridPoints(args);
        } else if (args[0] == "randomsample") {
		randomSample(args);
	} else if (args[0] == "create") {
		create(args);
	} else if (args[0] == "evaluate") {
		evaluateSurface(args);
	} else if (args[0] == "fitness" || args[0] == "error") {
		computeErrorMetric(args);
	} else if (args[0] == "convert") {
		conversion(args);
	} else {
		cout << "Unrecognized command" << endl;
	}
	
}

void executeScript(string filename) 
{
	cout << "Executing script: " << filename << endl;
	ifstream inscript(filename.c_str(), ios::in);
	if (!inscript) {
		cerr << "Error: file not found (" << filename << "). " << endl;
	} else {
		const int MAX_LINE = 200;
		char command[MAX_LINE];
		vector< string > args;
		while(!inscript.eof()) {
		    args.resize(0);
		    inscript.getline(command, MAX_LINE);
		    string commandstring = command;
		    if (commandstring != "\0") { // else it's an empty line; do nothing
		        istringstream incommand(commandstring);
		        while (!incommand.eof()) {
		            string nextarg;
		            incommand >> nextarg;
		            //cout << "Next arg: " << nextarg << endl;
		            args.push_back(nextarg);
		        }
		        //cout << "End of arguments " << endl;
                          
		        if (args.size() > 0) {
                          if (args[0][0] == '!') {
                             cout << "Ending execution" << endl;
                             break;
                          } else if (args[0][0] != '#') {
		             executeCommand(args);
                          }
		        }
		    }
		}
	}
}

void readEvalPrint()
{
	const int MAX_LINE = 200;
	//cout << "Read eval print" << endl;
	char command[MAX_LINE];
        vector< string > args;
	do {
	    args.resize(0);
	    cout << "surfpack> " << flush;
	    cin.getline(command, MAX_LINE);
	    string commandstring = command;
	    istringstream incommand(commandstring);
	    while (!incommand.eof()) {
                string nextarg;
		incommand >> nextarg;
		//cout << "Next arg: " << nextarg << endl;
		args.push_back(nextarg);
	    }
	    //cout << "End of arguments " << endl;
	    if (args.size() > 0 && args[0] != "quit") {
		    executeCommand(args);
	    }
	} while (! (args.size() > 0 && args[0] == "quit") );

	    
}
void printHelp()
{
  cout << "Usage: " << endl
       << "gridpoints <spec file> <output file>" << endl
       << "create <data file> <output file> <surface type> [<surface arguments> ...]" << endl
       << "evaluate <data file> <output file> <surface file>" << endl
       << "error <metric name> <surface file> " << endl
       << "convert <input file> <output file> " << endl;
}
       
       
int main(int argc, char* argv[]) 
{
  if (argc == 1) {
    readEvalPrint();
  } else if (argc == 2) {
    if (string(argv[1]) == "help") {
      printHelp();
    } else {     
      executeScript(argv[1]);
    }
  } else {
    vector< string > args;
    for (int i = 1; i < argc; i++) {
      args.push_back(argv[i]);
    }
    executeCommand(args);
  }
  return 0;
}    
