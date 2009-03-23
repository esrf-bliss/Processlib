
class BaseImageProcessClass
{
 public:
  virtual const Image::Header& prepare(const Image &aSrc) const = 0;
  virtual const Image& process(const Image &aSrc) = 0;
  virtual bool release() = 0;
};

