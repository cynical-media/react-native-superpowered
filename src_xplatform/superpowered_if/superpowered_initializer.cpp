#include "superpowered_initializer.hpp"
#include <Superpowered.h>
#include <SuperpoweredAdvancedAudioPlayer.h>

SpInit &SpInit::inst() {
  static SpInit theInst;
  return theInst;
}

void SpInit::init(const char *tmpDirectory) {
  if (nullptr != tmpDirectory){
    Superpowered::AdvancedAudioPlayer::setTempFolder(tmpDirectory);
  }
}

SpInit::~SpInit() {
}

SpInit::SpInit(){
#if 1
  Superpowered::Initialize(
      "ExampleLicenseKey-WillExpire-OnNextUpdate",
      false, // enableAudioAnalysis (using SuperpoweredAnalyzer, SuperpoweredLiveAnalyzer, SuperpoweredWaveform or SuperpoweredBandpassFilterbank)
      false, // enableFFTAndFrequencyDomain (using SuperpoweredFrequencyDomain, SuperpoweredFFTComplex, SuperpoweredFFTReal or SuperpoweredPolarFFT)
      true, // enableAudioTimeStretching (using SuperpoweredTimeStretching)
      true, // enableAudioEffects (using any SuperpoweredFX class)
      true,  // enableAudioPlayerAndDecoder (using SuperpoweredAdvancedAudioPlayer or SuperpoweredDecoder)
      false, // enableCryptographics (using Superpowered::RSAPublicKey, Superpowered::RSAPrivateKey, Superpowered::hasher or Superpowered::AES)
      true  // enableNetworking (using Superpowered::httpRequest)
  );
#endif
};
