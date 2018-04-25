

#include <processlib/TaskMgr.h>
#include <processlib/PoolThreadMgr.h>
#include <processlib/BackgroundSubstraction.h>
#include <processlib/Binning.h>
#include <processlib/RoiCounter.h>



int main()
{
	// Order of Data, Buffer and TaskMgr construction matters!
	Buffer *buffer= new Buffer(2048 * 2048);
	Data backGroundImg;
	TaskMgr task_mgr;

	// Background
	Tasks::BackgroundSubstraction *bckground = new Tasks::BackgroundSubstraction();
	backGroundImg.setBuffer(buffer);
	buffer->unref();
	backGroundImg.type = Data::INT8;
	backGroundImg.dimensions = { 2048, 2048 };
	//(2048, 2048), dtype = numpy.ushort)
	//backGroundData = processlib.Data()
	//backGroundData.buffer = backGroundImg;

	bckground->setBackgroundImageData(backGroundImg);
	task_mgr.setLinkTask(0, bckground);

	// Binning
	Tasks::Binning *binning = new Tasks::Binning();
	binning->mXFactor = 2;
	binning->mYFactor = 2;
	task_mgr.setLinkTask(1, binning);

	// Roi Counter
	Tasks::RoiCounterManager rc_mgr;
	Tasks::RoiCounterTask *RCTask = new Tasks::RoiCounterTask(rc_mgr);
	RCTask->setRoi(0, 0, 2048, 2048);
	task_mgr.addSinkTask(1, RCTask);

	// NOTE: TaskMgr takes ownership of the tasks pointer

	// And finnaly set the chain task for all input images
	auto poolThreadMgr = PoolThreadMgr::get();
	poolThreadMgr.setNumberOfThread(2);
	poolThreadMgr.setTaskMgr(&task_mgr);

	return 0;
}
