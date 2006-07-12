/*  _______________________________________________________________________
 
    Surfpack: A Software Library of Multidimensional Surface Fitting Methods
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
    For more information, see the README file in the top Surfpack directory.
    _______________________________________________________________________ */

#include "surfpack_config.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "unittests.h"
#include "surfpack.h"

using namespace std;

const std::string& dataRoot(const std::string* newroot)
{
  static string s("/tmp/mricha/SurfpackData");
  if (newroot) {
    s = *newroot;
  }
  return s;
}

const string fullPath(const string filename)
{
  return dataRoot() + "/" + filename;
}

void writePoint1Files()
{
  ofstream outfile(fullPath("point1.txt").c_str(), ios::out);
  outfile << "1.0 2.0 3.0 4.0" << endl;
  outfile.close();
  double vals[4];
  vals[0] = 1.0;
  vals[1] = 2.0;
  vals[2] = 3.0;
  vals[3] = 4.0;
  ofstream outfile2(fullPath("point1.sp").c_str(), ios::out | ios::binary);
  outfile2.write(reinterpret_cast<char*>(&vals),sizeof(double)*4);
  outfile2.close();

}

void writePoint2Files()
{
  ofstream outfile(fullPath("point2.txt").c_str(), ios::out);
  outfile << "0.0 1.0 -2.0" << endl;
  outfile.close();
  double vals[3];
  vals[0] = 0.0;
  vals[1] = 1.0;
  vals[2] = -2.0;
  ofstream outfile2(fullPath("point2.sp").c_str(), ios::out | ios::binary);
  outfile2.write(reinterpret_cast<char*>(&vals),sizeof(double)*3);
  outfile2.close();
}

void writeRastriginAndClaimsTooManyFiles()
{
  unsigned intvals[3];
  intvals[0] = 100;
  intvals[1] = 2;
  intvals[2] = 1;
  ofstream rastriginText(fullPath("rast100.spd").c_str(),ios::out);
  setOstreamFlags(rastriginText);
  ofstream rastriginBinary(fullPath("rast100.bspd").c_str(),
    ios::out | ios::binary);
  ofstream claimsTooManyText(fullPath("claimsTooMany.spd").c_str(),
    ios::out);
  setOstreamFlags(claimsTooManyText);
  ofstream claimsTooManyBinary(fullPath("claimsTooMany.bspd").c_str(),

    ios::out);
  
  // write SurfData headers
  rastriginText << intvals[0] << endl 
		<< intvals[1] << endl 
		<< intvals[2] << endl;
  claimsTooManyText << intvals[0]+1 << endl 
		    << intvals[1] << endl 
		    << intvals[2] << endl;
  rastriginBinary.write(reinterpret_cast<char*>(intvals),sizeof(unsigned)*3);
  intvals[0]++;
  claimsTooManyBinary.write(reinterpret_cast<char*>(intvals),sizeof(unsigned)*3);
  
  // write data
  double min = -2.0;
  double max = 2.0;
  unsigned ptsPerDim = 10;
  double interval = (max - min) / static_cast<double>(ptsPerDim - 1);
  vector<double> onept(2);
  double response = 0.0;
  for (unsigned dim1 = 0; dim1 < ptsPerDim; dim1++) {
    onept[0] = min + dim1 * interval;
    for (unsigned dim2 = 0; dim2 < ptsPerDim; dim2++) {
      onept[1] = min + dim2 * interval;
      response = surfpack::rastrigin(onept);
      rastriginText << setw(surfpack::field_width) << onept[0] 
		    << setw(surfpack::field_width) << onept[1]
		    << setw(surfpack::field_width) << response
		    << endl;
      claimsTooManyText << setw(surfpack::field_width) << onept[0] 
		        << setw(surfpack::field_width) << onept[1]
		        << setw(surfpack::field_width) << response
		        << endl;
      rastriginBinary.write(reinterpret_cast<char*>(&onept[0]),sizeof(double));
      rastriginBinary.write(reinterpret_cast<char*>(&onept[1]),sizeof(double));
      rastriginBinary.write(reinterpret_cast<char*>(&response),sizeof(double));
      claimsTooManyBinary.write(reinterpret_cast<char*>(&onept[0]),sizeof(double));
      claimsTooManyBinary.write(reinterpret_cast<char*>(&onept[1]),sizeof(double));
      claimsTooManyBinary.write(reinterpret_cast<char*>(&response),sizeof(double));
    }
  }     

  rastriginText.close();
  rastriginBinary.close(); 
  claimsTooManyText.close();
  claimsTooManyBinary.close();

}

void writeManyPtsFiles()
{
  unsigned intvals[3];
  intvals[0] = 10000;
  intvals[1] = 5;
  intvals[2] = 1;
  ofstream manyptsText(fullPath("manypts.spd").c_str(),ios::out);
  setOstreamFlags(manyptsText);
  ofstream manyptsBinary(fullPath("manypts.bspd").c_str(),
    ios::out | ios::binary);
  
  // write SurfData headers
  manyptsText << intvals[0] << endl 
		<< intvals[1] << endl 
		<< intvals[2] << endl;
  manyptsBinary.write(reinterpret_cast<char*>(intvals),sizeof(unsigned)*3);
  
  // write data
  double min = -2.0;
  double max = 2.0;
  unsigned ptsPerDim = 10;
  double interval = (max - min) / static_cast<double>(ptsPerDim - 1);
  vector<double> onept(5);
  double response = 0.0;
  for (unsigned dim0 = 0; dim0 < ptsPerDim; dim0++) {
    onept[0] = min + dim0 * interval;
    for (unsigned dim1 = 0; dim1 < ptsPerDim; dim1++) {
      onept[1] = min + dim1 * interval;
      for (unsigned dim2 = 0; dim2 < ptsPerDim; dim2++) {
        onept[2] = min + dim2 * interval;
        for (unsigned dim3 = 0; dim3 < ptsPerDim; dim3++) {
          onept[3] = min + dim3 * interval;
          for (unsigned dim4 = 0; dim4 < ptsPerDim; dim4++) {
            onept[4] = min + dim4 * interval;
	      response = surfpack::rastrigin(onept);
	      manyptsText << setw(surfpack::field_width) << onept[0] 
			    << setw(surfpack::field_width) << onept[1]
			    << setw(surfpack::field_width) << onept[2]
			    << setw(surfpack::field_width) << onept[3]
			    << setw(surfpack::field_width) << onept[4]
			    << setw(surfpack::field_width) << response
			    << endl;
      	     manyptsBinary.write(reinterpret_cast<char*>(&onept[0]),sizeof(double));
      	     manyptsBinary.write(reinterpret_cast<char*>(&onept[1]),sizeof(double));
      	     manyptsBinary.write(reinterpret_cast<char*>(&onept[2]),sizeof(double));
      	     manyptsBinary.write(reinterpret_cast<char*>(&onept[3]),sizeof(double));
      	     manyptsBinary.write(reinterpret_cast<char*>(&onept[4]),sizeof(double));
      	     manyptsBinary.write(reinterpret_cast<char*>(&response),sizeof(double));
          }
	}
      }
    }
  }     
  manyptsText.close();
  manyptsBinary.close(); 

}

void writeOneDimQuadratic()
{
  ofstream outfile(fullPath("oneDimQuadratic.spd").c_str(),ios::out);
  outfile << "7\n1\n1" << endl;
  outfile << "0.0 0.0" << endl;
  outfile << "1.0 1.0" << endl;
  outfile << "-1.0 1.0" << endl;
  outfile << "2.0 4.0" << endl;
  outfile << "-2.0 4.0" << endl;
  outfile << "3.0 9.0" << endl;
  outfile << "-3.0 9.0" << endl;
  outfile.close();
}

void writeOneDQpoly2Files()
{
  ofstream outfile(fullPath("oneDQpoly2.sps").c_str(),ios::out);
  outfile << "polynomial" << endl;
  outfile << "1 dimensions" << endl;
  outfile << "2 order" << endl;
  outfile << "0                          +" << endl;
  outfile << "0                         x1 +" << endl;
  outfile << "1                         x1^2" << endl;
  outfile << "0 response index for surface data" << endl;
  outfile << "7" << endl;
  outfile << "1" << endl;
  outfile << "1" << endl;
  outfile << "   0.00000000000000000e+00   0.00000000000000000e+00" << endl;
  outfile << "   1.00000000000000000e+00   1.00000000000000000e+00" << endl;
  outfile << "  -1.00000000000000000e+00   1.00000000000000000e+00" << endl;
  outfile << "   2.00000000000000000e+00   4.00000000000000000e+00" << endl;
  outfile << "  -2.00000000000000000e+00   4.00000000000000000e+00" << endl;
  outfile << "   3.00000000000000000e+00   9.00000000000000000e+00" << endl;
  outfile << "  -3.00000000000000000e+00   9.00000000000000000e+00" << endl;
  outfile.close();
  
  // write the same surface out in binary
  ofstream binoutfile(fullPath("oneDQpoly2.bsps").c_str(),ios::out|ios::binary);
  // write out the name
  string name = "polynomial";
  unsigned nameSize = name.length();
  binoutfile.write(reinterpret_cast<char*>(&nameSize),sizeof(nameSize));
  binoutfile.write(name.c_str(),nameSize);
  // write out size and order
  unsigned dim = 1;
  binoutfile.write(reinterpret_cast<char*>(&dim),sizeof(dim));
  unsigned polyorder = 2;
  binoutfile.write(reinterpret_cast<char*>(&polyorder),sizeof(polyorder));
  // write out coefficients
  double vals[3];
  vals[0] = 0.0;
  vals[1] = 0.0;
  vals[2] = 1.0;
  binoutfile.write(reinterpret_cast<char*>(&vals[0]),sizeof(double)*3);
  // write out response index
  unsigned responseIndex = 0;
  // write out data
  binoutfile.write(reinterpret_cast<char*>(&responseIndex),sizeof(unsigned));
  unsigned intvals[3];
  intvals[0] = 7;
  intvals[1] = 1;
  intvals[2] = 1;
  binoutfile.write(reinterpret_cast<char*>(&intvals[0]),sizeof(unsigned)*3);
  double dvals[14];
  dvals[0] = 0.0;
  dvals[1] = 0.0;
  dvals[2] = 1.0;
  dvals[3] = 1.0;
  dvals[4] = -1.0;
  dvals[5] = 1.0;
  dvals[6] = 2.0;
  dvals[7] = 4.0;
  dvals[8] = -2.0;
  dvals[9] = 4.0;
  dvals[10] = 3.0;
  dvals[11] = 9.0;
  dvals[12] = -3.0;
  dvals[13] = 9.0;
  binoutfile.write(reinterpret_cast<char*>(&dvals[0]),sizeof(double)*14);
  binoutfile.close(); 
}

void writeUnknownSurfaceFile()
{
  ofstream outfile(fullPath("unknown.sps").c_str(),ios::out);
  outfile << "Unknown" << endl;
  outfile << "1 dimensions" << endl;
  outfile << "2 order" << endl;
  outfile << "0                          +" << endl;
  outfile << "0                         x1 +" << endl;
  outfile << "1                         x1^2" << endl;
  outfile << "0 response index for surface data" << endl;
  outfile << "7" << endl;
  outfile << "1" << endl;
  outfile << "1" << endl;
  outfile << "   0.00000000000000000e+00   0.00000000000000000e+00" << endl;
  outfile << "   1.00000000000000000e+00   1.00000000000000000e+00" << endl;
  outfile << "  -1.00000000000000000e+00   1.00000000000000000e+00" << endl;
  outfile << "   2.00000000000000000e+00   4.00000000000000000e+00" << endl;
  outfile << "  -2.00000000000000000e+00   4.00000000000000000e+00" << endl;
  outfile << "   3.00000000000000000e+00   9.00000000000000000e+00" << endl;
  outfile << "  -3.00000000000000000e+00   9.00000000000000000e+00" << endl;
  outfile.close();
}

void setOstreamFlags(ostream& os)
{
    ios::fmtflags old_flags = os.flags();
    unsigned old_precision = os.precision(surfpack::output_precision);
    os.setf(ios::scientific);
}

void initialize()
{
  static bool initialized = false;
  if (!initialized) {
    ostringstream heredoc;
    heredoc << "sh <<MKDIREOF" << endl
 	    << "if test -d " << dataRoot()  << endl
 	    << "then" << endl
 	    << "  chmod 777 " << dataRoot() << endl
	    << "  echo Directory already exists " << endl
	    << "else" << endl
	    << "  mkdir -m 777 " << dataRoot() << endl
	    << "fi " << endl
 	    << "MKDIREOF" << endl;
    system(heredoc.str().c_str()); 
    initialized = true;
    cout << "Writing test files...." << endl;
    writePoint1Files();
    writePoint2Files();
    writeRastriginAndClaimsTooManyFiles();
    writeManyPtsFiles();
    writeOneDimQuadratic();
    writeOneDQpoly2Files();
    writeUnknownSurfaceFile();
    ostringstream permiss;
    permiss << "cd " << dataRoot() << "; chmod 666 ./*; cd -" << endl;
    system(permiss.str().c_str());
  }
}

void cleanup()
{
  cout << "Cleaning up test files...." << endl;
  ostringstream heredoc;
  heredoc << "sh <<RMDIREOF" << endl
          << "rm -rf " << dataRoot() << endl
          << "RMDIREOF" << endl;
  system(heredoc.str().c_str()); 
}

bool matches(double observed, double target, double margin)
{
  bool result;
  if (abs(target) < 1e-10) {
    result = abs(observed) < margin;
  } else {
    result = abs((observed - target) / target) < margin;
  }

  if (result) return true;
  cerr << "Test Value: " << observed << " Expected: " << target << endl;
  return false;
}
