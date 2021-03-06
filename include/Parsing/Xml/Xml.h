/////////
//	Xml Structure (Generation 5)
//	Copyright (C) 2010-2022 by Wiley Black
////

#ifndef __WBXml_v5_h__
#define __WBXml_v5_h__

/** Table of Contents **/

namespace wb
{
	namespace xml
	{
		//struct _XmlSerializationData;

		class XmlAttribute;
		class XmlNode;
			class XmlDocument;
			class XmlElement;
			class XmlText;
	}
}

/**	Dependencies **/

#include <string>
#include <vector>
#include "../../Foundation/Exceptions.h"

/**	Dependencies **/

namespace wb
{
	namespace xml
	{
		using namespace std;

		/** XmlWriterOptions **/

		/// <summary>XmlWriterOptions provides a set of control options for generating a string or stream from XML content.</summary>
		class XmlWriterOptions
		{
		public:
			/// <summary>[Default=false]  Indicates that children and inner content should be included.</summary>
			bool	IncludeContent;

			/// <summary>[Default=0]  Indentation level for output text.</summary>
			int		Indentation;

			/// <summary>[Default=true]  Allows the use of single tag elements when there are no children.  For example, &lt;Example /%gt; would
			/// be allowed.  If false, the element would be written as %lt;Example%gt;%lt;/Example%gt;.</summary>
			bool	AllowSingleTags; 

			/// <summary>[Default=false]  Normally whitespace is permitted in attribute values without requiring escaping, but this option
			/// will force all whitespace to be escaped.</summary>
			bool	EscapeAttributeWhitespace;
			
			XmlWriterOptions(XmlWriterOptions&);
			XmlWriterOptions(XmlWriterOptions&&) noexcept;
			XmlWriterOptions(bool AndContent = true);
		};

		/** JsonWriterOptions **/

		/// <summary>JsonWriterOptions provides a set of control options for generating a JSON string or stream from XML content.</summary>
		class JsonWriterOptions
		{
		public:			
			/// <summary>[Default=0]  Indentation level for output text.</summary>
			int		Indentation;						
			
			/// <summary>[Default=false] Allows the merging of multiple JSON "arrays" that would be created from the existence of repeated
			/// element names but are not necessarily contiguous.  In XML, there is no concept of an array but elements can be repeated.  In
			/// JSON an array is a specific thing and the only way to properly repeat a value under the same name.  Therefore, there is not
			/// a strict translation from XML to JSON in the case where an XML element tag gets repeated but with other tags interspersed.
			/// In some formats, the sequencing can have meaning.  To enable this option potentially discards the XML sequencing by merging
			/// all instances of the same XML element tag into the same array, discarding the information that there were other XML nodes
			/// interspersed.  If this option if false, an error is generated instead of re-sequencing.</summary>
			bool	MergeArrays;

			JsonWriterOptions() { Indentation = 0; MergeArrays = false; }
		};

		/** XmlAttribute **/

		/// <summary>XmlAttribute represents an in-memory attribute from XML.  For example, the node &lt;sample name=&quot;Wiley&quot;&gt; contains
		/// a single attribute with Name "name" and Value "Wiley".</summary>
		class XmlAttribute
		{
		public:
			string	Name;
			string	Value;

			XmlAttribute() { }
			XmlAttribute(const XmlAttribute& cp) { Name=cp.Name; Value=cp.Value; }
		};

		/** XmlNode **/

		/// <summary>The XmlNode class contains the base for all xml node classes, including XmlElement (the most common node).  XmlDocument is
		/// also based on XmlNode and provides the top-level container for a document or snippet.</summary>
		class XmlNode
		{
		protected:
			XmlNode();
			virtual ~XmlNode();
			
			friend class	XmlParser;	
			friend class	XmlElement;
			static string	Escape(const XmlWriterOptions& Options, const string&);
			static string	Escape(const JsonWriterOptions& Options, const string&);
			static void		Indent(const XmlWriterOptions& Options, string& OnString);
			static void		Indent(const JsonWriterOptions& Options, string& OnString);			
			virtual string	ToJsonValue(JsonWriterOptions Options = JsonWriterOptions());

		public:	

			vector<shared_ptr<XmlNode>>	Children;

			/// <summary>
			/// Elements() provides a vector containing references to all children that are XmlElements.
			/// </summary>
			const vector<shared_ptr<XmlElement>>	Elements() const;

				/** FindChild(): Returns the first child element matching the specified tag name.
					Returns NULL if no child elements match the tag name.  Descends exactly 
					one level only (in other words, grandchildren and siblings are not considered.) **/
			shared_ptr<XmlElement> FindChild(const string& TagName);

				/** FindNthChild(): Behaves the same as FindChild(), but returns the Nth occurrance
					of the tag name in the children list.  Returns NULL if fewer than (N+1) child elements
					match the tag name. **/
			shared_ptr<XmlElement> FindNthChild(const string& TagName, int N);
			
				/** Appends the specified node at the end of the list of child nodes. **/
			void AppendChild(shared_ptr<XmlNode> pNewChild);
			void AppendChild(shared_ptr<XmlElement>& pNewChild) { AppendChild(dynamic_pointer_cast<XmlNode>(pNewChild)); }
			void AppendChild(shared_ptr<XmlText>& pNewChild) { AppendChild(dynamic_pointer_cast<XmlNode>(pNewChild)); }

				/** Removes the node matching the pointer given.  The memory associated with pChild is
					not freed, however responsibility for freeing it (with delete) becomes that of the
					caller. **/
			void RemoveChild(shared_ptr<XmlNode> pChild);
			void RemoveChild(shared_ptr<XmlElement>& pChild) { RemoveChild(dynamic_pointer_cast<XmlNode>(pChild)); }
			void RemoveChild(shared_ptr<XmlText>& pChild) { RemoveChild(dynamic_pointer_cast<XmlNode>(pChild)); }

				/** Returns true if there are child nodes contained in this XmlNode **/
			bool HasChildNodes() { return Children.size() > 0; }

			enum_class_start(Type,int)
			{
				Document,
				Element,
				Text
			}
			enum_class_end(Type);

			virtual bool	IsElement() { return false; }		// Returns true only if this XmlNode is of type XmlElement.			
			virtual Type	GetType() = 0;
			
			virtual string	ToString(XmlWriterOptions Options = XmlWriterOptions());
			virtual string	ToJson(JsonWriterOptions Options = JsonWriterOptions());

			/// <summary>Creates a deep copy of this node, all children, and all attributes.  The caller assumes 
			/// responsibility for delete'ing the returned node.  The returned copy is not attached to any
			/// higher-level hierarchy and has no parent.</summary>
			virtual shared_ptr<XmlNode>	DeepCopy() = 0;

			/// <summary>Available for more information during exceptions, the SourceLocation can be set by the parser to
			/// later identify where an XmlNode originated from.  Note that an XmlNode created in code will not have a
			/// SourceLocation and the string will be empty, and that providing a source file to the parser is also optional
			/// and SourceLocation may identify only a line number.  When a source file is provided, the parser will 
			/// typically use the format &lt;SourceFile&gt;:&lt;LineNumber&gt; such as SourceFile.xml:35.</summary>
			string SourceLocation;
		};

		/** XmlDocument **/		

		/// <summary>Represents the top-level xml container from a document or snippet.</summary>
		class XmlDocument : public XmlNode
		{		
		public:
			XmlDocument() { }
			~XmlDocument() { }	

			string ToString(XmlWriterOptions Options = XmlWriterOptions()) override;
			string ToJson(JsonWriterOptions Options = JsonWriterOptions()) override;

				/** GetDocumentElement() returns the top-level (root) element of 
					this Xml document.  NULL is returned if the top-level element
					does not exist.
				**/
			shared_ptr<XmlElement>	GetDocumentElement();

			XmlNode::Type	GetType() { return XmlNode::Type::Document; }

			shared_ptr<XmlNode>		DeepCopy() override;
		};

		/** XmlElement **/

		/// <summary>XmlElement represents any xml element, which is a node that can contain other xml nodes and/or xml attributes.</summary>
		class XmlElement : public XmlNode
		{
		protected:			
			friend class	XmlParser;	
			friend class	XmlDocument;
			string	ToJsonValue(JsonWriterOptions Options = JsonWriterOptions()) override;

		public:
			XmlElement(const string& LocalName = "");
			XmlElement(const XmlElement&) = delete;			// Use DeepCopy instead.
			~XmlElement();

			string			LocalName;

			vector<shared_ptr<XmlAttribute>>	Attributes;

				/** GetAttribute(): Returns the value for the attribute with the specified name.
					Returns an empty string if the attribute is not found. **/
			string			GetAttribute(const string& AttrName) const;

				/** FindAttribute(): Returns the attribute with the specified name.  Returns NULL
					if the attribute is not found. **/
			shared_ptr<XmlAttribute>	FindAttribute(const string& AttrName) const;

				/** GetAs...(pszAttrName, [Default Value]) helpers:
					A series of convenient attribute conversion/access/find functions.  Each call 
					checks that the node consists of XmlText child(ren), which is (are) taken as the
					value if found.  If the node contains any XmlElement children, a FormatException is
					thrown.  If there are no children, the string is assumed empty, which will cause an
					exception for any conversion except GetTextAsString().  If conversion fails, a 
					FormatException is thrown. **/
			string			GetTextAsString() const;
			int				GetTextAsInt32() const;
			unsigned int	GetTextAsUInt32() const;
			Int64			GetTextAsInt64() const;
			UInt64			GetTextAsUInt64() const;
			float			GetTextAsFloat() const;
			double			GetTextAsDouble() const;
			bool  			GetTextAsBool() const;

				/** GetAttrAs...(pszAttrName, [Default Value]) helpers:
					A series of convenient attribute conversion/access/find functions.  Each searches
					for the named attribute.  If it is not found, the default value is returned.  If it
					is found, it is converted to the type of the function and returned.  If conversion
					fails, a FormatException is thrown. **/
			string			GetAttrAsString(const string& AttrName, const string& DefaultValue = "") const;
			int				GetAttrAsInt32(const string& AttrName, int DefaultValue = 0) const;
			unsigned int	GetAttrAsUInt32(const string& AttrName, unsigned int DefaultValue = 0) const;
			Int64			GetAttrAsInt64(const string& AttrName, Int64 DefaultValue = 0) const;
			UInt64			GetAttrAsUInt64(const string& AttrName, UInt64 DefaultValue = 0) const;
			float			GetAttrAsFloat(const string& AttrName, float DefaultValue = 0.0) const;
			double			GetAttrAsDouble(const string& AttrName, double DefaultValue = 0.0) const;
			bool  			GetAttrAsBool(const string& AttrName, bool DefaultValue = false) const;
			/**
			complex<double>	GetAttrAsComplex(const char *pszAttrName, complex<double> cDefaultValue ) const;
			complex<double>	GetAttrAsComplex(const char *pszAttrName ) const { complex<double> cZero = 0.; return GetAsComplex(pszAttrName,cZero); }
			**/
			bool			IsAttrPresent(const string& AttrName ) const;

				/** Add...AsAttr(pszAttrName, [Value]) helpers:
					A series of convenient attribute creation functions.  Each appends a new attribute
					to the end of the attribute list after converting the attribute into a string
					format. **/			
			void			AddStringAsAttr(const string& AttrName, const string& Value);
			void			AddInt32AsAttr(const string& AttrName, int Value);
			void			AddUInt32AsAttr(const string& AttrName, unsigned int Value);
			void			AddInt64AsAttr(const string& AttrName, Int64 Value);
			void			AddUInt64AsAttr(const string& AttrName, UInt64 Value);
			void			AddFloatAsAttr(const string& AttrName, float Value);
			void			AddDoubleAsAttr(const string& AttrName, double Value);
			void			AddBoolAsAttr(const string& AttrName, bool Value);

				/** Set...AsAttr(pszAttrName, [Value]) helpers:
					A series of attribute editing functions.  Each searches for the attribute, and
					if it is found, replaces its value with that given.  If the attribute is not
					present, it is added with the given value. **/
			void			SetStringAsAttr(const string& AttrName, const string& sValue);
			void			SetInt32AsAttr(const string& AttrName, int Value);
			void			SetUInt32AsAttr(const string& AttrName, unsigned int Value);
			void			SetInt64AsAttr(const string& AttrName, Int64 Value);
			void			SetUInt64AsAttr(const string& AttrName, UInt64 Value);
			void			SetFloatAsAttr(const string& AttrName, float Value);
			void			SetDoubleAsAttr(const string& AttrName, double Value);
			void  			SetBoolAsAttr(const string& AttrName, bool Value);

				/** Add...AsText(pszName, [Value]) helpers:
					A series of convenient element creation functions.  Each appends a new element
					to the end of the children of this node containing the textual form of the
					value. **/			
			void			AddStringAsText(const string& Name, const string& Value);
			void			AddInt32AsText(const string& Name, int Value);
			void			AddUInt32AsText(const string& Name, unsigned int Value);
			void			AddInt64AsText(const string& Name, Int64 Value);
			void			AddUInt64AsText(const string& Name, UInt64 Value);
			void			AddFloatAsText(const string& Name, float Value);
			void			AddDoubleAsText(const string& Name, double Value);
			void			AddBoolAsText(const string& Name, bool Value);

				/** Add...([Value]) helpers:
					A series of convenient functions.  Each creates an XmlText child
					at the end of the children of this node containing the textual form of the
					value. **/
			void			AddString(const string& Value);
			void			AddInt32(int Value);
			void			AddUInt32(unsigned int Value);
			void			AddInt64(Int64 Value);
			void			AddUInt64(UInt64 Value);
			void			AddFloat(float Value);
			void			AddDouble(double Value);
			void			AddBool(bool Value);

			bool			IsElement() { return true; }
			XmlNode::Type	GetType() { return XmlNode::Type::Element; }

			string			ToString(XmlWriterOptions Options = XmlWriterOptions()) override;
			string			ToJson(JsonWriterOptions Options = JsonWriterOptions()) override;
			shared_ptr<XmlNode>		DeepCopy() override;
		};

		/** XmlText **/

		/// <summary>XmlText represents a section of text as an xml node.  The text must be contained as a child within an
		/// XmlElement.</summary>
		class XmlText : public XmlNode
		{		
		protected:
			string	ToJsonValue(JsonWriterOptions Options = JsonWriterOptions()) override;

		public:	
			XmlText() { }
			~XmlText() { }

			/// <summary>Text provides the in-memory representation of the node.  That is, there are no escaped characters
			/// in Text.  Escaping of characters is performed by ToString(), and unescaping is performed during parsing.</summary>
			string Text;

			XmlNode::Type	GetType() { return XmlNode::Type::Text; }

			string			ToString(XmlWriterOptions Options = XmlWriterOptions()) override;
			string			ToJson(JsonWriterOptions Options = JsonWriterOptions()) override;
			shared_ptr<XmlNode>		DeepCopy() override;

			static string	Escape(string RegularText);
			static string	Unescape(string EscapedText);
		};
	}
}

/** Late dependencies **/

#include "Implementation/XmlImpl.h"

#endif	// __WBXml_v5_h__

//	End of Xml.h

