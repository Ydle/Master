/*
 * RestBrowser.h
 *
 *  Created on: Jan 6, 2014
 *      Author: denia
 */

#ifndef RESTBROWSER_H_
#define RESTBROWSER_H_

#include "jsoncpp/json/json.h"

namespace ydle {

class RestBrowser {
public:
	RestBrowser(std::string url);
	virtual ~RestBrowser();
	Json::Value doGet(std::string url, std::string parameters);
	Json::Value doPost(std::string url, Json::Value  *post_data);
	Json::Value doPost(std::string url, std::string  post_data);
	// Callback function for handle the data received from a request
	static size_t responseToJsonObjectCallback( char *ptr, size_t size, size_t nmemb, void *userdata);
private:

	std::string url;
};

} /* namespace ydle */

#endif /* RESTBROWSER_H_ */
