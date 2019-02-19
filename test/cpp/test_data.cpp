#define BOOST_TEST_MODULE data
#include <boost/test/unit_test.hpp>

#include <processlib/Data.h>

BOOST_AUTO_TEST_CASE(test_data)
{
    Data beam_data;
    beam_data.frameNumber = 1;
    beam_data.dimensions = { 2048, 2048 };
    beam_data.type = Data::INT8;

    Buffer *buffer = new Buffer(2048 * 2048);

    //Transfer ownership to Data
    beam_data.setBuffer(buffer);
    buffer->unref();
    
    BOOST_CHECK_EQUAL(0, 0);
}
