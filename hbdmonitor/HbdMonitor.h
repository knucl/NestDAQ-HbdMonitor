#ifndef Example_Sink_h
#define Example_Sink_h

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#if __has_include(<fairmq/Device.h>)
#include <fairmq/Device.h> // since v1.4.34
#else
#include <fairmq/FairMQDevice.h>
#endif

//#include "mydecoder/LeafProcessor.hh"
#include "LeafProcessor.hh"

class HbdMonitor : public FairMQDevice {
public:

  struct OptionKey {
    static constexpr std::string_view InputChannelName{ "in" };
    //static constexpr std::string_view Multipart{ "multipart" };
  };

  HbdMonitor() = default;
  HbdMonitor(const HbdMonitor&) = delete;
  HbdMonitor &operator=(const HbdMonitor&) = delete;
  ~HbdMonitor() = default;

private:

  bool HandleMultipartData( FairMQParts& msgParts, int index );
  void InitTask() override;
  void UnpackMessage( FairMQParts& inParts );

  std::string fInputChannelName;
  uint64_t fNumMessages{ 0 };
  nestdaq::unpacker::LeafProcessor fLeafProcessor;
  int32_t fShowCounter{ 0 };

};


#endif
