#ifndef __BPM_H__
#define __BPM_H__

#include <list>
#include <vector>
#include "Task.h"

namespace Tasks
{
  class BpmManager
  {
  public:
    class Result
    {
      friend class BpmManager;
    public:
      enum ErrorCode {OK,NO_MORE_AVAILABLE,TIMEDOUT};
      Result():
	beam_intensity(0),
	beam_center_x(0.),
	beam_center_y(0.),
	beam_fwhm_x(0.),
	beam_fwhm_y(0.),
	max_pixel_value(0),
	max_pixel_x(0),
	max_pixel_y(0),
	frameNumber(-1),
	AOI_automatic(true),
	AOI_extension_factor(4),
	AOI_min_x(-1),
	AOI_max_x(-1),
	AOI_min_y(-1),
	AOI_max_y(-1),
	border_exclusion(10),
	errorCode(OK)
      {}

      double		beam_intensity;
      double		beam_center_x;
      double		beam_center_y;
      double		beam_fwhm_x;
      int		beam_fwhm_min_x_index;
      int		beam_fwhm_max_x_index;
      double		beam_fwhm_y;
      int		beam_fwhm_min_y_index;
      int		beam_fwhm_max_y_index;
      unsigned int	max_pixel_value;
      unsigned int	max_pixel_x;
      unsigned int	max_pixel_y;

      int		frameNumber;
      
      bool		AOI_automatic;
      unsigned int	AOI_extension_factor;
      int		AOI_min_x;
      int		AOI_max_x;
      int		AOI_min_y;
      int		AOI_max_y;
      unsigned int	border_exclusion;
      ErrorCode		errorCode;

    private:
      explicit Result(ErrorCode code) :
	errorCode(code) {}
      void _setAOI(const int aRoi[])
      {
	AOI_min_x = aRoi[0];
	AOI_max_x = aRoi[1];
	AOI_min_y = aRoi[2];
	AOI_max_y = aRoi[3];
	AOI_automatic = false;
      }

    };
    
    class BpmTask : public Task
    {
    public:
      BpmTask(BpmManager &);
      BpmTask(const BpmTask&);
      virtual Data process(Data&); 
      virtual Task* copy() const;
      
      bool		mFwhmTunning;
      double		mFwhmTunningExtension;
      double		mAoiExtension;
      int		mBorderExclusion;
      unsigned int	mThreshold;
      bool		mEnableX;
      bool		mEnableY;
      bool		mEnableBackgroundSubstration;
      bool		mRoiAutomatic;
      
      double		mBackgroundLevelx;
      double		mBackgroundLevely;
      double		mBackgroundLevelTunex;
      double		mBackgroundLevelTuney;
    private:
      BpmManager &_mgr;
      
      template<class IN>
      void _treat_image(const Data &aSrc,
	                Buffer &projection_x,
		        Buffer &projection_y,
		        Result &aResult);
      template<class IN> void _tune_projection(const Data &aSrc,
					       Buffer&,Buffer&,
					       const Result&);
      double _calculate_fwhm(const Buffer &projection,int size,
			     int max_index,double background_level,
			     int &min_index,int &max_index);
      void _calculate_background(const Buffer &projection,double &background_level,
				 int min_index,int max_index);
    };
    friend class BpmTask;
    enum RUN_MODE {Parallel,Sequential};
    explicit BpmManager(int historySize = 4);
    ~BpmManager();
    
    void	setMode(RUN_MODE);
    BpmTask*	getBpmTask();
    Result	getResult(double timeout = -1.,
			  int frameNumber = -1) const;
    void	getHistory(std::list<Result> &aHistory) const;
    void	resizeHistory(int aSize);
  private:
    double			_lastBackgroundLevelx;
    double			_lastBackgroundLevely;
    double			_lastBackgroundLevelTunex;
    double			_lastBackgroundLevelTuney;

    int				_currentFrameNumber;
    mutable pthread_mutex_t	_lock;
    mutable pthread_cond_t	_cond;
    std::vector<Result>		_historyResult;
    RUN_MODE			_mode;
    inline bool _isFrameAvailable(int aFrameNumber) const;
    void _getLastBackgroundLevel(BpmTask &aResult) const;
    void _setResult(const Result&,const BpmTask&);
  };
  inline bool operator<(const BpmManager::Result &A,const BpmManager::Result &B) 
  { return A.frameNumber < B.frameNumber;}
}
#endif
