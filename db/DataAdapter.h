#ifndef  _XPOKER_SRC_DATA_ADAPTER_H_
#define _XPOKER_SRC_DATA_ADAPTER_H_

#include <assistx2/json_wrapper.h>
#include <google/protobuf/message.h>

class IQueryResult;

namespace poker
{
	std::int32_t JsonToMessage(json_spirit::Value & json, ::google::protobuf::Message * msg);

	void MessageToJson(::google::protobuf::Message * msg, json_spirit::Value & json);

	void DataBaseToJson(IQueryResult * result, const ::google::protobuf::Descriptor * descriptor, json_spirit::Value & json);

}

#endif

