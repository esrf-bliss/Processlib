namespace Tasks
{
 class BpmManager
  {	
%TypeHeaderCode
#include <Bpm.h>
%End
    public:
    enum ErrorCode {OK,NO_MORE_AVAILABLE,TIMEDOUT};
    enum RUN_MODE {Counter,Monitor};
    BpmManager(int = 4);
    ~BpmManager();

    Tasks::BpmResult	getResult(double = 0.,int = -1) const;
    void	resizeHistory(int aSize);
    void  reset();
    int	  historySize() const;
    int lastFrameNumber() const;
  };

  class BpmResult
    {
%TypeHeaderCode
#include <Bpm.h>
%End
    public:
      BpmResult();

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
      Tasks::BpmManager::ErrorCode errorCode;
    };

 

 class BpmTask : SinkTaskBase
  {
%TypeHeaderCode
#include <Bpm.h>
%End
  public:
    BpmTask(Tasks::BpmManager &);
    BpmTask(const Tasks::BpmTask&);
    virtual void process(Data&); 
      
    bool		mFwhmTunning;
    double		mFwhmTunningExtension;
    double		mAoiExtension;
    int			mBorderExclusion;
    unsigned int	mThreshold;
    bool		mEnableX;
    bool		mEnableY;
    bool		mEnableBackgroundSubstration;
    bool		mRoiAutomatic;
  };


};
