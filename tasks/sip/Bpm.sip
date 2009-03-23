namespace Tasks
{
  class BpmManager
  {
%TypeHeaderCode
#include <Bpm.h>
%End

  public:
    class Result
    {
%TypeHeaderCode
#include <Bpm.h>
%End
    public:
      enum ErrorCode {OK,NO_MORE_AVAILABLE,TIMEDOUT};
      Result();

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
    };
    
    class BpmTask : Task
    {
%TypeHeaderCode
#include <Bpm.h>
%End
    public:
      BpmTask(Tasks::BpmManager &);
      virtual Data process(Data&); 

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
    };
    BpmManager(int = 4);
    ~BpmManager();

    BpmTask*	getBpmTask();
    Result	getResult(double = -1.,int = -1) const;
    void	resizeHistory(int aSize);
  };
};