#include "DataAdapter.h"

#include <stdexcept>

#include <json_spirit_writer_template.h>
#include <json_spirit_reader_template.h>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <assistx2/database_wrapper.h>

namespace poker
{
		std::int32_t JsonToMessage(json_spirit::Value & json, ::google::protobuf::Message * msg)
		{
			if (json.type() != json_spirit::array_type)
			{
				LOG(INFO) << "Failed Message name:=" << msg->GetDescriptor()->name()
					<< ", json:=" << json_spirit::write_string(json);

				return -1;
			}

			auto descriptor = msg->GetDescriptor();

			auto & array = json.get_array();
			//DCHECK_EQ(array.size(), static_cast<size_t>(descriptor->field_count()) );
			if (array.size() != static_cast<size_t>(descriptor->field_count() ) )
			{
				LOG(INFO) << "Failed Message name:=" << msg->GetDescriptor()->name()
					<< ", json:=" << json_spirit::write_string(json);

				return -2;
			}

			const google::protobuf::Reflection * reflection = msg->GetReflection();
			DCHECK_NOTNULL(reflection);

			for (int i = 0; i < descriptor->field_count(); ++i)
			{
				const google::protobuf::FieldDescriptor * field_descriptor = descriptor->field(i);
				DCHECK_NOTNULL(field_descriptor);

				// ÉèÖÃÖµ
				switch (field_descriptor->type())
				{
				case ::google::protobuf::FieldDescriptor::TYPE_STRING:
				{
					reflection->SetString(msg, field_descriptor, assistx2::ToString(array.at(i)));
// 					DLOG(INFO) << "name:=" << field_descriptor->name() << ", str:=" << assistx2::ToString(array.at(i)) << ", index:=" << i
// 						<< ", " << json_spirit::write_string(json);
				}
					break;
				case ::google::protobuf::FieldDescriptor::TYPE_FIXED32:
				case ::google::protobuf::FieldDescriptor::TYPE_INT32:
					reflection->SetInt32(msg, field_descriptor, assistx2::ToInt(array.at(i)));
					break;
				case ::google::protobuf::FieldDescriptor::TYPE_FIXED64:
				case ::google::protobuf::FieldDescriptor::TYPE_INT64:
					reflection->SetInt64(msg, field_descriptor, assistx2::ToInt64(array.at(i)));
					break;
				default:
					DCHECK(false) << "UNKNOWN type:=" << field_descriptor->type();
					break;
				}
			}

			return 0;
		}

		void MessageToJson(::google::protobuf::Message * msg, json_spirit::Value & json)
		{
			const google::protobuf::Reflection * reflection = msg->GetReflection();
			DCHECK_NOTNULL(reflection);

			auto descriptor = msg->GetDescriptor();

			json_spirit::Array array;

			for (int i = 0; i < descriptor->field_count(); ++i)
			{
				const google::protobuf::FieldDescriptor * field_descriptor = descriptor->field(i);
				DCHECK_NOTNULL(field_descriptor);

				switch (field_descriptor->type())
				{
				case ::google::protobuf::FieldDescriptor::TYPE_STRING:
				{
					array.push_back(reflection->GetString(*msg, field_descriptor) );

//					auto str  = reflection->GetString(*msg, field_descriptor);
// 					json_spirit::Value  value;
// 					if (json_spirit::read(str, value) == false )
// 					{
// 						array.push_back(str);
// 					}
// 					else
// 					{
// 						array.push_back(value);
// 					}
				}
					break;
				case ::google::protobuf::FieldDescriptor::TYPE_FIXED32:
				case ::google::protobuf::FieldDescriptor::TYPE_INT32:
				{
					array.push_back(assistx2::ToInt(array.at(i) ) );
				}
					break;
				case ::google::protobuf::FieldDescriptor::TYPE_FIXED64:
				case ::google::protobuf::FieldDescriptor::TYPE_INT64:
					array.push_back(assistx2::ToInt64(array.at(i) ) );
					break;
				default:
					DCHECK(false) << "UNKNOWN type:=" << field_descriptor->type();
					break;
				}
			}
		}

		void DataBaseToJson(IQueryResult * result, const ::google::protobuf::Descriptor * descriptor, json_spirit::Value & json)
		{
			json_spirit::Array array;

			for (int i = 0; i < descriptor->field_count(); ++i)
			{
				const google::protobuf::FieldDescriptor * field_descriptor = descriptor->field(i);
				DCHECK_NOTNULL(field_descriptor);

				switch (field_descriptor->type())
				{
				case ::google::protobuf::FieldDescriptor::TYPE_STRING:
				{
					array.push_back(result->GetItemString(0, field_descriptor->name()) );
                    
					//DLOG(INFO) << "name:=" << field_descriptor->name() << ", str:=" << str << ", index:=" << i;
						//<<", message:="<< descriptor->

// 					json_spirit::Value  value;
// 					if (json_spirit::read(str, value) == false)
// 					{
// 						DLOG(INFO) << "+++++ name:=" << field_descriptor->name() << ", str:=" << str;
// 					}
// 					else
// 					{
// 						DLOG(INFO) << "---------- name:=" << field_descriptor->name() << ", value:=" << json_spirit::write_string(value)
// 							<< ", str:=" << str << ", " << value.type();
// 						array.push_back(value);
// 					}
				}
				break;
				case ::google::protobuf::FieldDescriptor::TYPE_FIXED32:
				case ::google::protobuf::FieldDescriptor::TYPE_INT32:
				case ::google::protobuf::FieldDescriptor::TYPE_FIXED64:
				case ::google::protobuf::FieldDescriptor::TYPE_INT64:
					array.push_back(result->GetItemLong(0, field_descriptor->name()));
					break;
				default:
					DCHECK(false) << "UNKNOWN type:=" << field_descriptor->type();
					break;
				}
			}

			json = array;
		}
}

