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

	Tasks::BpmManager bpm_manager;
	Tasks::BpmTask bpm_task(bpm_manager);

	bpm_task.mEnableX = true;
	bpm_task.mEnableY = true;
	// bpm_task.mThreshold = 42;
	bpm_task.process(beam_data);
	auto result = bpm_manager.getResult();

		//print("beam_intensity", result.beam_intensity)
		//print("beam_center_x", result.beam_center_x)
		//print("beam_center_y", result.beam_center_y)
		//print("beam_fwhm_x", result.beam_fwhm_x)
		//print("beam_fwhm_min_x_index", result.beam_fwhm_min_x_index)
		//print("beam_fwhm_max_x_index", result.beam_fwhm_max_x_index)
		//print("beam_fwhm_y", result.beam_fwhm_y)
		//print("beam_fwhm_min_y_index", result.beam_fwhm_min_y_index)
		//print("beam_fwhm_max_y_index", result.beam_fwhm_max_y_index)
		//print("max_pixel_value", result.max_pixel_value)
		//print("max_pixel_x", result.max_pixel_x)
		//print("max_pixel_y", result.max_pixel_y)

		//print("frameNumber", result.frameNumber)

		//print("AOI_automatic", result.AOI_automatic)
		//print("AOI_extension_factor", result.AOI_extension_factor)
		//print("AOI_min_x", result.AOI_min_x)
		//print("AOI_max_x", result.AOI_max_x)
		//print("AOI_min_y", result.AOI_min_y)
		//print("AOI_max_y", result.AOI_max_y)
		//print("border_exclusion", result.border_exclusion)



		return 0;
}