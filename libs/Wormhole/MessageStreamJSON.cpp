/* Copyright 2013 David Axmark

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file MessageStreamJSON.cpp
 * @author Mikael Kindborg
 *
 * Class for parsing JSON messages from a WebView.
 */

#include <ma.h>				// MoSync API
#include <maheap.h>			// C memory allocation
#include <mastring.h>		// C string functions
#include <mavsprintf.h>		// C string functions
#include <mastdlib.h>		// C string conversion functions
#include <conprint.h>

#include "MessageStreamJSON.h"

using namespace MAUtil;

namespace Wormhole
{
	/**
	 * Constructor. Here we parse the message.
	 */
	MessageStreamJSON::MessageStreamJSON(
		MAWidgetHandle webViewHandle,
		MAHandle dataHandle)
	{
		mWebViewHandle = webViewHandle;
		mWebView = NULL;
		mCurrentMessageIndex = -1;
		parse(dataHandle);
	}

	/**
	 * Constructor.
	 * @deprecated
	 */
	MessageStreamJSON::MessageStreamJSON(
		NativeUI::WebView* webView,
		MAHandle dataHandle)
	{
		mWebViewHandle = webView->getWidgetHandle();
		mWebView = webView;
		mCurrentMessageIndex = -1;
		parse(dataHandle);
	}

	/**
	 * Destructor. Here we delete the JSON tree.
	 */
	MessageStreamJSON::~MessageStreamJSON()
	{
		// The root must not be NULL or Value::NUL.
		if (NULL != mJSONRoot && YAJLDom::Value::NUL != mJSONRoot->getType())
		{
			// Delete the JSON tree.
			YAJLDom::deleteValue(mJSONRoot);
			mJSONRoot = NULL;
		}
	}

	/**
	 * Get the WebView widget handle associated with this message.
	 * @return Handle to WebView widget.
	 */
	MAWidgetHandle MessageStreamJSON::getWebViewHandle()
	{
		return mWebViewHandle;
	}

	/**
	 * Get the WebView object associated with this message.
	 * @return WebView object.
	 * @deprecated
	 */
	NativeUI::WebView* MessageStreamJSON::getWebView()
	{
		return mWebView;
	}

	/**
	 * Evaluate JavaScript the WebView associated with this message.
	 * @param script JavaScript string.
	 */
	void MessageStreamJSON::callJS(const MAUtil::String& script)
	{
		MAUtil::String url = "javascript:" + script;
		maWidgetSetProperty(
			mWebViewHandle,
			MAW_WEB_VIEW_URL,
			url.c_str());
	}

	/**
	 * Move to the next message. Initially, the message
	 * pointer is positioned right before the first message.
	 * Do a call to this method to get the first message.
	 *
	 * @return true if moved to next message, false
	 * if there are no more messages.
	 */
	bool MessageStreamJSON::next()
	{
		if (NULL != mJSONRoot && YAJLDom::Value::ARRAY == mJSONRoot->getType())
		{
			++mCurrentMessageIndex;
			return mCurrentMessageIndex < mJSONRoot->getNumChildValues();
		}
		return false;
	}

	/**
	 * Checks if this message matches the given message name.
	 */
	bool MessageStreamJSON::is(const char* paramName)
	{
		YAJLDom::Value* value = getParamNode("messageName");
		if (NULL != value && YAJLDom::Value::STRING == value->getType())
		{
		// Commented out code that was intended to be used with
		// pointers into the unparsed JSON data. This was made as
		// part of a project to remove creation of String objects
		// from YAJLDom. But this was never finished. A problem
		// with the raw string data is that it needs to be unstringified,
		// and therefore one of the benefits of using raw data gets lost,
		// you still need to create a new string. Code left here as a
		// reference.
//			YAJLDom::StringValue* stringValue = (YAJLDom::StringValue*) value;
//			return 0 == strncmp(
//				paramName,
//				stringValue->getCharPointer(),
//				stringValue->getLength());
			return value->toString() == paramName;
		}
		return false;
	}

	/**
	 * Returns the string value of a top-level message parameter.
	 * @return The param value as a string.
	 */
	String MessageStreamJSON::getParam(const char* paramName)
	{
		YAJLDom::Value* value = getParamNode(paramName);
		if (NULL != value && YAJLDom::Value::STRING == value->getType())
		{
			return value->toString();
		}
		return "";
	}

	/**
	 * Returns the integer value of a top-level message parameter.
	 * @return The param value as an int.
	 */
	int MessageStreamJSON::getParamInt(const char* paramName)
	{
		YAJLDom::Value* value = getParamNode(paramName);
		if (NULL != value && YAJLDom::Value::NUMBER == value->getType())
		{
			return value->toInt();
		}
		return 0;
	}

	/**
	 * Checks if the given top-level parameter name is in the message.
	 */
	bool MessageStreamJSON::hasParam(const char* paramName)
	{
		YAJLDom::Value* value = getParamNode(paramName);
		return (NULL != value && YAJLDom::Value::NUL != value->getType());
	}

	/**
	 * Get the node of a top-level parameter in the current message.
	 */
	YAJLDom::Value* MessageStreamJSON::getParamNode(const char* paramName)
	{
		if (NULL != mJSONRoot && YAJLDom::Value::ARRAY == mJSONRoot->getType())
		{
			YAJLDom::Value* message = mJSONRoot->getValueByIndex(mCurrentMessageIndex);
			if (YAJLDom::Value::MAP == message->getType())
			{
				return message->getValueForKey(paramName);
			}
		}
		return NULL;
	}

	/**
	 * @return The JSON root node.
	 */
	MAUtil::YAJLDom::Value* MessageStreamJSON::getJSONRoot()
	{
		return mJSONRoot;
	}

	/**
	 * Parse the message. This finds the message name and
	 * creates a dictionary with the message parameters.
	 */
	void MessageStreamJSON::parse(MAHandle dataHandle)
	{
		//lprintfln("@@@ MessageStreamJSON::parse %i", dataHandle);

		// We must have data.
		if (NULL == dataHandle)
		{
			return;
		}

		// Get length of the data, it is not zero terminated.
		int dataSize = maGetDataSize(dataHandle);

		// Allocate buffer for string data.
		char* stringData = (char*) malloc(dataSize + 1);

		// Get the data.
		maReadData(dataHandle, stringData, 0, dataSize);

		// Zero terminate.
		stringData[dataSize] = 0;

		//maWriteLog(stringData, dataSize);

		// Check that we have the "ma:" prefix,
		// followed by the JSON array.
		if (stringData[0] != 'm') { return; }
		if (stringData[1] != 'a') { return; }
		if (stringData[2] != ':') { return; }
		if (stringData[3] != '[') { return; }

		// Pointer to the start of the JSOn array at the
		// opening '[' character.
		char* jsonData = stringData + 3;

		mJSONRoot = YAJLDom::parse(
			(const unsigned char*)jsonData,
			dataSize - 3);

		free(stringData);
	}

} // namespace
