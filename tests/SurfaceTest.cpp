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
#include "SurfaceTest.h"
#include "SurfData.h"
#include "Surface.h"
#include "unittests.h"
#include "PolynomialSurface.h"
#include "SurfScaler.h"

using std::cout;
using std::endl;
using std::ostringstream;
using std::string;
using std::vector;

CPPUNIT_TEST_SUITE_REGISTRATION( SurfaceTest );

void SurfaceTest::setUp()
{
  surfd = new SurfData(string("oneDimQuadratic.spd"));
  polysurf = new PolynomialSurface(surfd,2);
}

void SurfaceTest::tearDown()
{
  delete surfd;
  delete polysurf;
}

void SurfaceTest::xSize()
{
  vector<double> cs;
  PolynomialSurface ps(4,3,cs);
  CPPUNIT_ASSERT( ps.xSize() == 4 );
  PolynomialSurface ps2( 0, 2);
  CPPUNIT_ASSERT_EQUAL( (unsigned)0, ps2.xSize());
  SurfData sd(string("oneDimQuadratic.spd"));
  PolynomialSurface ps3(&sd,2);
  CPPUNIT_ASSERT_EQUAL( (unsigned)1, ps3.xSize() );
  ps3.setData(0);
  // We don't want the xSize reset to zero just because
  // the data may have gone away.  It could still be a valid
  // surface.
  CPPUNIT_ASSERT_EQUAL( (unsigned)1, ps3.xSize() );
} 

void SurfaceTest::hasOriginalData()
{
  vector<double> cs;
  PolynomialSurface ps(4,3,cs);
  CPPUNIT_ASSERT( !ps.hasOriginalData() );
  PolynomialSurface ps2( 0, 2);
  CPPUNIT_ASSERT( !ps2.hasOriginalData() );
  SurfData sd(string("oneDimQuadratic.spd"));
  PolynomialSurface ps3(&sd,2);
  CPPUNIT_ASSERT( !ps3.hasOriginalData() );
  ps3.createModel();
  CPPUNIT_ASSERT( ps3.hasOriginalData() );
  ps3.setData(0);
  CPPUNIT_ASSERT( !ps3.hasOriginalData() );
  PolynomialSurface* ps4 = ps3.makeSimilarWithNewData(&sd);
  CPPUNIT_ASSERT( ps4->hasOriginalData() );
  delete ps4;
}

void SurfaceTest::acceptableData()
{
  SurfData sd(string("oneDimQuadratic.spd"));
  PolynomialSurface ps(&sd,2);
  CPPUNIT_ASSERT( ps.acceptableData() );
}

void SurfaceTest::acceptableDataExceptionNull()
{
  SurfData sd(string("oneDimQuadratic.spd"));
  PolynomialSurface ps(0,1);
  ps.acceptableData();
}

void SurfaceTest::acceptableDataExceptionNotEnough()
{
  SurfData sd(string("oneDimQuadratic.spd"));
  PolynomialSurface ps(&sd,7);
  ps.acceptableData();
}


void SurfaceTest::getValueVector()
{
  vector<double> x(1);
  x[0] = 3.0;
  CPPUNIT_ASSERT(matches(polysurf->getValue(x),9.0));
  PolynomialSurface ps(surfd, 2);
  ps.createModel();
  CPPUNIT_ASSERT(matches(ps.getValue(x),9.0));
}

void SurfaceTest::getValueVectorScaled()
{
  vector<double> x(1);
  x[0] = 3.0;
  CPPUNIT_ASSERT(matches(polysurf->getValue(x),9.0));
  PolynomialSurface ps(surfd, 2);
  ps.scaleUniform();
  ps.createModel();
  ps.write(string("badscale.sps"));
  cout << ps.scaler->asString();
  CPPUNIT_ASSERT(matches(ps.getValue(x),9.0));
}

void SurfaceTest::getValueSurfPoint()
{
  vector<double> x(1);
  x[0] = -4.0;
  SurfPoint sp(x);
  CPPUNIT_ASSERT(matches(polysurf->getValue(sp),16.0));

}

void SurfaceTest::getValueSurfPointScaled()
{
  vector<double> x(1);
  x[0] = -4.0;
  SurfPoint sp(x);
  polysurf->scaleUniform();
  polysurf->createModel();
  CPPUNIT_ASSERT(matches(polysurf->getValue(sp),16.0));

}

void SurfaceTest::getValueSurfData()
{
  polysurf->getValue(*surfd);
  CPPUNIT_ASSERT(surfd->fSize() == 2);
  CPPUNIT_ASSERT(matches((*surfd)[1].F(1),1.0));

}

void SurfaceTest::noScale()
{
  CPPUNIT_ASSERT( polysurf->scaler == 0);
  polysurf->noScale();
  CPPUNIT_ASSERT( polysurf->scaler == 0);
  polysurf->scaleUniform();
  CPPUNIT_ASSERT( polysurf->scaler != 0);
  polysurf->noScale();
  CPPUNIT_ASSERT( polysurf->scaler == 0);
}

void SurfaceTest::scaleUniform()
{
  polysurf->scaleUniform();
  polysurf->createModel();
  CPPUNIT_ASSERT( 1==polysurf->scaler->scalers.size());
  int loc = polysurf->scaler->scalers[0]->asString().find("offset: -3");
  CPPUNIT_ASSERT(loc >= 0 && loc < polysurf->scaler->scalers[0]->asString().size());
  loc = polysurf->scaler->scalers[0]->asString().find("divisor: 6");
  CPPUNIT_ASSERT(loc >= 0 && loc < polysurf->scaler->scalers[0]->asString().size());
}

void SurfaceTest::getValueErrorStructs()
{
  vector<surfpack::ErrorStruct> es;
  polysurf->getValue(*surfd, es);
  CPPUNIT_ASSERT(es.size() == 7);
  CPPUNIT_ASSERT(matches(es[0].estimated, es[0].observed));
}

void SurfaceTest::goodnessOfFit()
{
  // The purpose of this test is only to make sure the flow
  // of execution does not produce an exception under one
  // set of normal circumstances.  Accuracy of the metrics
  // should be examined in other tests
  polysurf->goodnessOfFit("press",0);
  polysurf->goodnessOfFit("rsquared",0);
  polysurf->goodnessOfFit("sum_squared",0);
  polysurf->goodnessOfFit("mean_squared",0);
  polysurf->goodnessOfFit("max_relative",0);
}
 
void SurfaceTest::goodnessOfFitException()
{
  polysurf->goodnessOfFit("__no_such_metric__",0);
}

void SurfaceTest::press()
{
  CPPUNIT_ASSERT(matches(polysurf->goodnessOfFit("press",0), 0.0));
}

void SurfaceTest::pressExceptionInsufficient()
{
  PolynomialSurface ps2(surfd, 6);
  CPPUNIT_ASSERT(matches(ps2.goodnessOfFit("press",0), 0.0));
}

void SurfaceTest::rSquared()
{
  CPPUNIT_ASSERT(matches(polysurf->goodnessOfFit("rsquared",0), 1.0));
}

void SurfaceTest::mse()
{
  CPPUNIT_ASSERT(matches(polysurf->goodnessOfFit("mean_squared",0), 0.0));
}

void SurfaceTest::sse()
{
  CPPUNIT_ASSERT(matches(polysurf->goodnessOfFit("sum_squared",0), 0.0));
}

void SurfaceTest::mrae()
{
  CPPUNIT_ASSERT(matches(polysurf->goodnessOfFit("max_relative",surfd), 0.0));
}

void SurfaceTest::checkDataException()
{
  vector<double> coefficients(3);
  PolynomialSurface ps(1,2,coefficients);
  ps.goodnessOfFit("press",0);
}

void SurfaceTest::createModelSurfData()
{
  vector<double> coefficients;
  PolynomialSurface ps(1,2,coefficients);
  ps.createModel(surfd);
  vector<double> x(1);
  x[0] = 2.5;
  CPPUNIT_ASSERT(matches(ps.getValue(x),6.25));
}

void SurfaceTest::recreateModel()
{
  vector<double> coefficients;
  PolynomialSurface ps(1,2,coefficients);
  ps.createModel(surfd);
  ps.createModel();
}

void SurfaceTest::writeNoFile()
{
  polysurf->write("///.sps");
}

void SurfaceTest::writeNoDataText()
{
  vector<double> coefficients(3);
  coefficients[0] = 0.0;
  coefficients[1] = 0.0;
  coefficients[2] = 1.0;
  PolynomialSurface ps(1,2,coefficients);
  ps.write(string("poly2NoData.sps"));
}

void SurfaceTest::writeNoDataBinary()
{
  vector<double> coefficients(3);
  coefficients[0] = 0.0;
  coefficients[1] = 0.0;
  coefficients[2] = 1.0;
  PolynomialSurface ps(1,2,coefficients);
  ps.write(string("poly2NoData.bsps"));
}

void SurfaceTest::readNoFile()
{
  PolynomialSurface ps("__file_does_not__exist.sps");
}

void SurfaceTest::readBadName()
{
  PolynomialSurface ps(string("unknown.sps"));

}

void SurfaceTest::badFileExtension()
{
  PolynomialSurface ps(string("unknown.krt"));
}

void SurfaceTest::print()
{
  // only checking for non-crashing behavior
  ostringstream os;
  os << *polysurf << endl;
  cout << "End SurfaceTest" << endl;
}

