/////////
//	Yaml.h
////

#ifndef __WBYaml_h__
#define __WBYaml_h__

/** Table of Contents **/

namespace wb
{
	namespace yaml
	{		
		class YamlNode;
		class YamlScalar;
		class YamlSequence;
		class YamlMapping;
	}
}

/** Dependencies **/

#include "../../wbFoundation.h"
#include "../../Foundation/STL/Collections/UnorderedMap.h"
#include "../../Foundation/STL/Collections/Map.h"
#include "../../IO/Streams.h"
#include "../../IO/MemoryStream.h"

/** Content **/

namespace wb
{
	namespace yaml
	{
		using namespace std;				

		/** JsonWriterOptions **/

		/// <summary>JsonWriterOptions provides a set of control options for generating a JSON string or stream from YAML content.</summary>
		class JsonWriterOptions
		{
		public:
			/// <summary>[Default=0]  Indentation level for output text.</summary>
			int		Indentation;

			/// <summary>[Default=false] Ordinarily, all scalar content is quoted.  If true, recognize and write numeric scalar content without
			/// the quotes.
			/// </summary>
			bool	UnquoteNumbers;

			JsonWriterOptions() { Indentation = 0; UnquoteNumbers = false; }
		};

		/** YamlNode **/

		class YamlNode
		{			
		protected:
			static void		AddIndent(const JsonWriterOptions& Options, string& OnString) 
			{
				for (int ii = 0; ii < Options.Indentation; ii++) OnString += '\t';
			}

		public:
			YamlNode(string FromSource) : Source(FromSource), Tag("?") { }
			virtual ~YamlNode() { }

			string Tag;
			string Source;

			YamlNode& operator=(const YamlNode&) = default;
			virtual unique_ptr<YamlNode>	DeepCopy() = 0;

			virtual string	ToJson(JsonWriterOptions Options = JsonWriterOptions())
			{
				throw NotImplementedException("Conversion to Json value for node at " + Source + " was not implemented.");
			}
		};

		/** YamlScalar **/

		class YamlScalar : public YamlNode
		{
			typedef YamlNode base;
			friend class YamlSequence;
			friend class YamlMapping;

			string	ToJsonValue(bool UnquoteNumbers)
			{
				if (!UnquoteNumbers) return "\"" + wb::json::JsonString::Escape(Content) + "\"";

				// Check if the value is entirely numeric...
				for (auto ii = 0; ii < Content.length(); ii++)
				{
					if ((Content[ii] >= '0' && Content[ii] <= '9') || toupper(Content[ii]) == 'E' || toupper(Content[ii]) == '.'
						|| Content[ii] == '+' || Content[ii] == '-') continue;
					return "\"" + wb::json::JsonString::Escape(Content) + "\"";
				}
				// It is entirely numeric.  Omit quotes.
				return Content;
			}

		public:			
			YamlScalar(string FromSource, string Text = "") : base(FromSource), Content(Text) { }

			YamlScalar(const YamlScalar&) = default;
			YamlScalar& operator=(const YamlScalar&) = default;

			string	Content;
			
			unique_ptr<YamlNode>	DeepCopy() override {
				return unique_ptr<YamlNode>(new YamlScalar(*this));
			}

			string	ToJson(JsonWriterOptions Options = JsonWriterOptions()) override
			{
				string ret;								
				ret += ToJsonValue(Options.UnquoteNumbers);
				return ret;
			}
		};

		/** YamlSequence **/

		class YamlSequence : public YamlNode
		{
			typedef YamlNode base;
		public:			
			YamlSequence(string FromSource) : base(FromSource) { }
			
			YamlSequence(const YamlSequence&) = delete;
			YamlSequence& operator=(const YamlSequence&) = delete;

			vector<unique_ptr<YamlNode>>	Entries;

			unique_ptr<YamlNode>	DeepCopy() override {
				auto pRet = make_unique<YamlSequence>(Source);
				pRet->base::operator=(*this);		// Shallow copy the base members
				for (auto& it : Entries) {
					if (!it) pRet->Entries.push_back(nullptr);
					else pRet->Entries.push_back(it->DeepCopy());
				}
				return dynamic_pointer_movecast<YamlNode>(std::move(pRet));
			}

			string	ToJson(JsonWriterOptions Options = JsonWriterOptions()) override
			{
				string ret;				
				ret += "[\n";
				Options.Indentation++;
				bool First = true;
				for (auto& pNode : Entries)
				{
					if (!First) ret += ",\n";
					else First = false;

					AddIndent(Options, ret);
					/*
					if (is_type<YamlScalar>(pNode))
					{						
						ret += ((YamlScalar*)pNode.get())->ToJsonValue(Options.UnquoteNumbers);
					}
					else
					{
					*/
					if (pNode == nullptr) ret += "null";
					else ret += pNode->ToJson(Options);
					//}
				}
				ret += "\n";
				Options.Indentation--;
				AddIndent(Options, ret);				
				ret += "]";
				return ret;
			}
		};

		/** YamlMapping **/

		class YamlMapping : public YamlNode
		{
			typedef YamlNode base;
		public:
			YamlMapping(string FromSource) : base(FromSource) { }

			unordered_map<unique_ptr<YamlNode>, unique_ptr<YamlNode>>	Map;			

			void Add(unique_ptr<YamlNode>&& pFrom, unique_ptr<YamlNode>&& pTo)
			{
				auto it = Map.find(pFrom);
				if (it != Map.end()) throw FormatException("Duplicate keys found at " + pFrom->Source + " and " + it->first->Source + " are not permitted in mapping at " + Source + ".");
				Map.insert(make_pair<unique_ptr<YamlNode>, unique_ptr<YamlNode>>(std::move(pFrom), std::move(pTo)));
			}			

			unique_ptr<YamlNode>	DeepCopy() override {
				auto pRet = make_unique<YamlMapping>(Source);
				pRet->base::operator=(*this);		// Shallow copy the base members				
				for (auto& it : Map) {
					if (it.first == nullptr && it.second == nullptr)
						pRet->Map.insert(make_pair<unique_ptr<YamlNode>, unique_ptr<YamlNode>>(nullptr, nullptr));
					else if (it.first == nullptr)
						pRet->Map.insert(make_pair<unique_ptr<YamlNode>, unique_ptr<YamlNode>>(nullptr, it.second->DeepCopy()));
					else if (it.second == nullptr)
						pRet->Map.insert(make_pair<unique_ptr<YamlNode>, unique_ptr<YamlNode>>(it.first->DeepCopy(), nullptr));
					else
						pRet->Map.insert(make_pair<unique_ptr<YamlNode>, unique_ptr<YamlNode>>(it.first->DeepCopy(), it.second->DeepCopy()));
				}
				return dynamic_pointer_movecast<YamlNode>(std::move(pRet));
			}

			string	ToJson(JsonWriterOptions Options = JsonWriterOptions()) override
			{
				string ret;
				ret += "{\n";
				Options.Indentation++;
				bool First = true;
				for (auto& KVP : Map)
				{
					if (!First) ret += ",\n";
					else First = false;

					/*
					if (is_type<YamlScalar>(KVP.first))
					{
						AddIndent(Options, ret);
						ret += ((YamlScalar*)KVP.first.get())->ToJsonValue(Options.UnquoteNumbers);
					}
					else
					{
					*/
					AddIndent(Options, ret);
					if (KVP.first == nullptr) ret += "\"\"";		// JSON does not permit a null key, so there is no perfect representation of the YAML here.
					else ret += KVP.first->ToJson(Options);
					//}

					ret += ": ";

					/*
					if (is_type<YamlScalar>(KVP.second))
					{
						AddIndent(Options, ret);
						ret += ((YamlScalar*)KVP.second.get())->ToJsonValue(Options.UnquoteNumbers);
					}
					else
					{
					*/
					if (KVP.second == nullptr) ret += "null";
					else ret += KVP.second->ToJson(Options);
					//}
				}
				ret += "\n";
				Options.Indentation--;
				AddIndent(Options, ret);
				ret += "}";				
				return ret;
			}
		};
	}
}

#endif	// __WBYaml_h__

//	End of Yaml.h


