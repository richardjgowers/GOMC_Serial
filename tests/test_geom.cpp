#include <boost/test/unit_test.hpp>
#include "../lib/GeomLib.h"

using namespace geom;

BOOST_AUTO_TEST_CASE(rad_to_def)
{
  BOOST_CHECK(RadToDeg(0.0) == 0.0);
}
