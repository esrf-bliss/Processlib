#include <processlib/Bpm.h>

int main()
{
	Data beam_data;
	beam_data.frameNumber = 1;
	beam_data.dimensions = { 2048, 2048 };
	beam_data.type = Data::INT8;

	Buffer *buffer = new Buffer(2048 * 2048);
	
	//Transfer ownership to Data
	beam_data.setBuffer(buffer);
	buffer->unref();

	return 0;
}


//int main()
//{
//	Data beam_data;
//	beam_data.frameNumber = 1;
//	beam_data.dimensions = { 2048, 2048 };
//	beam_data.type = Data::INT8;
//
//	Buffer buffer(2048 * 2048);
//
//	//Transfer ownership to Data
//	beam_data.setBuffer(&buffer);
//	buffer.unref();
//
//	return 0;
//}