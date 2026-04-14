#include <chrono>
#include <thread>
#include <iostream>

#include <fairmq/runFairMQDevice.h>

#include "HbdMonitor.h"

//#include "mydecoder/NodeHeaderSchema.hh"
//#include "mydecoder/NodeHeaderDecoder.hh"
//#include "mydecoder/LeafSchema.hh"
//#include "mydecoder/LeafDecoder.hh"
//#include "mydecoder/LeafProcessor.hh"
#include "NodeHeaderSchema.hh"
#include "NodeHeaderDecoder.hh"
#include "LeafSchema.hh"
#include "LeafDecoder.hh"
#include "LeafProcessor.hh"

namespace
{
  static const std::string kClassName{ "HbfMonitor" };
};

namespace bpo = boost::program_options;

//_____________________________________________________________________________
void addCustomOptions(bpo::options_description &options)
{
  using opt = HbdMonitor::OptionKey;
  options.add_options()
    ( opt::InputChannelName.data(), bpo::value<std::string>()->default_value( opt::InputChannelName.data() ), "Name of input channel\n" );
}

//_____________________________________________________________________________
FairMQDevicePtr getDevice( const FairMQProgOptions& )
{
  return new HbdMonitor;
}

//_____________________________________________________________________________
void PrintConfig( const fair::mq::ProgOptions* config, std::string_view name, std::string_view funcname )

{
  auto c = config->GetPropertiesAsStringStartingWith(name.data());
  std::ostringstream ss;
  ss << funcname << "\n\t " << name << "\n";
  for (const auto &[k, v] : c) {
      ss << "\t key = " << k << ", value = " << v << "\n";
  }
  LOG(debug) << ss.str();
}

//_____________________________________________________________________________
void HbdMonitor::InitTask()
{
  PrintConfig( fConfig, "channel-config", __PRETTY_FUNCTION__ );
  PrintConfig( fConfig, "chans.", __PRETTY_FUNCTION__ );

  using opt = OptionKey;

  fInputChannelName = fConfig->GetProperty<std::string>(opt::InputChannelName.data());
  LOG( debug ) << kClassName << "::InitTask() input channel = " << fInputChannelName;

  OnData( fInputChannelName, &HbdMonitor::HandleMultipartData );

  // LeafProcessor initialize //
  fLeafProcessor.clear();

  fShowCounter = 0;
}

//_____________________________________________________________________________
bool
HbdMonitor::HandleMultipartData( FairMQParts& msgParts, int index )
{
  static const std::string kFuncName( "[" + kClassName + "::" + __func__ + "()] " );

  namespace ndu = nestdaq::unpacker;

  fLeafProcessor.clear_node_block();
  fLeafProcessor.clear_hbd();

  HbdMonitor::UnpackMessage( msgParts );

  for( uint32_t i = 0; i < fLeafProcessor.get_num_frame(); ++i ) {
    fLeafProcessor.set_frame_index( i );
    fLeafProcessor.decode_heartbeat_delimiter();
  }

  auto&& log = fLeafProcessor.flush_log();
  if( log != "" ) LOG( warn ) << log;

  if( 0 == ( fShowCounter % 1000 ) ) {
    LOG( info ) << "DAQ system efficiency: " << fLeafProcessor.get_system_eff();
    auto node_effs = fLeafProcessor.get_node_effs();
    for( auto& [address, eff] : node_effs ) {
      LOG( info ) << " - Leaf node(" << address << ") eff: " << eff;
    }
  }

  ++fShowCounter;

  return true;

}

//_____________________________________________________________________________
void HbdMonitor::UnpackMessage( FairMQParts& inParts )
{
  static const std::string kFuncName( "[" + kClassName + "::" + __func__ + "()] " );

  namespace ndu = nestdaq::unpacker;

  ndu::DecodedHeaderData header_data;

  for( const auto& vmsg : inParts ) {
    uint64_t* ptr = reinterpret_cast< uint64_t* >( vmsg->GetData() );

    auto schema_page = nestdaq::g_header_schema.find( ptr[0] );
    if( nestdaq::g_header_schema.end() == schema_page ) {
      LOG( warn ) << kFuncName << "Unkown data block";
      LOG( warn ) << std::hex << ptr[0];
    } else {
      const auto [name, flags, hdef] = schema_page->second;
      if( true
        && flags[nestdaq::PageFlag::kIsLeaf]
        && flags[nestdaq::PageFlag::kIsBuilt] ) {
        ndu::decode_header( hdef, ptr, header_data );
        auto body_first = &ptr[hdef.size()];
        auto body_last  = &ptr[header_data["Length"]/sizeof( uint64_t )-1];

        fLeafProcessor.set_leaf_node( header_data, body_first, body_last );
      } else {
        ndu::decode_header( hdef, ptr, header_data );
      }

    }
  }
}
