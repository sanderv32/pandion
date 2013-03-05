/**
 * This file is part of Pandion.
 *
 * Pandion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pandion is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pandion.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Filename:    XMPPXMLParser.h
 * Author(s):   Dries Staelens
 * Copyright:   Copyright (c) 2009 Dries Staelens
 * Description: This file declares a parser for parsing XMPP XML data and 
 *              generating appropriate events.
 */

#pragma once
#include "XMPPLogger.h"
#include "XMPPHandlers.h"
#include "UTF.h"

/*
 * Enumeration containing all the possible parser states.
 */
typedef enum 
{ 
	ParsingProlog,
	ParsingRootElement,
	ParsingRootElementEnd,
	ParsingXMPPStanzaBegin,
	ParsingXMPPStanza,
	ParsingXMPPStanzaEnd
} XMPPXMLParsingState;

/*
 * Enumeration containing all the possible states for parsing the root element.
 */
typedef enum
{
	ParsingBeforeElement,
	ParsingName,
	ParsingBetweenAttributes,
	ParsingAttributeName,
	ParsingAttributeValue,
} XMPPXMLElementParsingState;

/*
 * The class that abstracts the XMPP XML parser.
 */
class XMPPXMLParser
{
	/*
	 * Reference to the DOMDocument pointer
	 */
	MSXML2::IXMLDOMDocumentPtr				xmlDoc;

	/*
	 * Reference to the object for handling the events generated by the parser.
	 */
	XMPPHandlers&							m_Handlers;
	/*
	 *
	 */
	XMPPLogger&								m_Logger;
	/*
	 * This field holds the current state of the parser.
	 */
	XMPPXMLParsingState						m_ParsingState;
	/*
	 * This field holds the current state while parsing a root element.
	 */
	XMPPXMLElementParsingState				m_ElementParsingState;
	/*
	 * This string acts as a buffer that stores the information already
	 * processed by the parser.
	 */
	UTF32String								m_ParsedData;
	/*
	 * This map holds a cache of corresponding namespaces for each element.
	 * This is not correctly implemented yet.
	 */
	std::map<UTF32String, UTF32String>		m_NamespaceCache;
	/*
	 * This map holds the attributes of the an element.
	 */
	std::map<UTF32String, UTF32String>		m_ElementAttributes;
	/*
	 * The element name curently being parsed.
	 */
	UTF32String								m_ElementName;
	/*
	 * The attribute name currently being parsed.
	 */
	UTF32String								m_AttributeName;
	/*
	 *  The attribute value currently being parsed.
	 */
	UTF32String								m_AttributeValue;
	/*
	 * The delimiter used by the attribute.
	 */
	unsigned								m_AttributeDelimiter;
	/*
	 * This holds the nesting level of the XMMP stanza being processed.
	 */
	int										m_ElementLevel;

	/*
	 * Constructor, destructor and public methods.
	 */
public:
	XMPPXMLParser(XMPPHandlers&, XMPPLogger&);
	~XMPPXMLParser();

	void SetConnected();
	void SetDisconnected();

	bool ParseChar(unsigned c);

	/*
	 * Private methods used for parsing the XMPP XML data.
	 */
private:
	void RestartParser();

	bool inline IsXMLWhitespace(unsigned xmlCharacter);
	void BuildNamespaceCache();

	bool ParseProlog(unsigned xmlCharacter);
	bool ParseRootElement(unsigned xmlCharacter);
	bool ParseRootElementEnd(unsigned xmlCharacter);
	bool ParseXMPPStanzaBegin(unsigned xmlCharacter);
	bool ParseXMPPStanza(unsigned xmlCharacter);
	bool ParseXMPPStanzaEnd(unsigned xmlCharacter);

	bool ParseElementBegin(unsigned xmlCharacter);
	bool ParseElementName(unsigned xmlCharacter);
	bool ParseBetweenAttributes(unsigned xmlCharacter);
	bool ParseAttributeName(unsigned xmlCharacter);
	bool ParseAttributeValue(unsigned xmlCharacter);

	bool HandleXMPPStanza();
	void FixXMLNS();
	bool NotifyHandler(const UTF32String& stanzaName);

	UTF32String::size_type GetStanzaEnd();
};