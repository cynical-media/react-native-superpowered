#ifndef SUPERPOWERED_INITIALIZER_HPP
#define SUPERPOWERED_INITIALIZER_HPP
#ifdef __cplusplus

class SpInit {
public:
  static SpInit &inst();
  void init(const char *tmpDirectory = nullptr);
  ~SpInit();
private:
  SpInit();
};
#endif

#endif
