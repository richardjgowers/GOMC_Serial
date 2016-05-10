#include <boost/test/unit_test.hpp>
#include "../lib/GeomLib.h"

using namespace geom;

BOOST_AUTO_TEST_CASE(rad_to_deg)
{
  BOOST_CHECK_CLOSE(RadToDeg(0.0), 0.0, 0.001);
  BOOST_CHECK_CLOSE(RadToDeg(3.14159265359), 180.0, 0.001);
  BOOST_CHECK_CLOSE(RadToDeg(- 3.14159265359), - 180.0, 0.001);
}
