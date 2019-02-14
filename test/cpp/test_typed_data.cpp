#define BOOST_TEST_MODULE typed_data
#include <boost/test/unit_test.hpp>

#include <processlib/TypedData.h>

BOOST_AUTO_TEST_CASE(test_buffer)
{
    TypedData<uint8_t> data;
    
	BOOST_CHECK_EQUAL(0, 0);
}
