

#include <processlib/TaskMgr.h>
#include <processlib/PoolThreadMgr.h>
#include <processlib/BackgroundSubstraction.h>
#include <processlib/Binning.h>
#include <processlib/RoiCounter.h>



int main()
{
	// Order of Data, Buffer and TaskMgr construction matters!
	Buffer *backgroundBuffer = new Buffer(2048 * 2048);
	TaskMgr *task_mgr = new TaskMgr();

	// Background
	Tasks::BackgroundSubstraction *bckground = new Tasks::BackgroundSubstraction();
	
	Data backgroundData;
	backgroundData.setBuffer(backgroundBuffer);
	backgroundBuffer->unref();	
	backgroundData.type = Data::INT8;
	backgroundData.dimensions = { 2048, 2048 };
	bckground->setBackgroundImageData(backgroundData);
	task_mgr->setLinkTask(0, bckground);

	// Binning
	Tasks::Binning *binning = new Tasks::Binning();
	binning->mXFactor = 2;
	binning->mYFactor = 2;
	task_mgr->setLinkTask(1, binning);

	// Roi Counter
	Tasks::RoiCounterManager rc_mgr;
	Tasks::RoiCounterTask *RCTask = new Tasks::RoiCounterTask(rc_mgr);
	RCTask->setRoi(0, 0, 2048, 2048);
	task_mgr->addSinkTask(1, RCTask);

	// NOTE: TaskMgr takes ownership of the tasks pointer

	// And finnaly set the chain task for all input images
	PoolThreadMgr& poolThreadMgr = PoolThreadMgr::get();
	poolThreadMgr.setNumberOfThread(2);
	

	Buffer *inputBuffer = new Buffer(2048 * 2048);
	Data inputData;
	inputData.setBuffer(inputBuffer);
	inputBuffer->unref();
	inputData.type = Data::INT8;
	inputData.dimensions = { 2048, 2048 };
	task_mgr->setInputData(inputData);
	poolThreadMgr.addProcess(task_mgr);

	poolThreadMgr.wait(5);

	return 0;
}
