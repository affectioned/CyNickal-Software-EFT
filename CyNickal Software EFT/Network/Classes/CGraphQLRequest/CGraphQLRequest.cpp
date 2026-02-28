#include "pch.h"
#include "sqlite3.h"
#include "json.hpp"
#include "CGraphQLRequest.hpp"
#include "Network/Callbacks/Callbacks.hpp"

CGraphQLRequest::CGraphQLRequest(const std::string& URL, const std::string& Query)
{
	auto curl = curl_easy_init();
	if (!curl)
	{
		std::println("[CGraphQLRequest] curl_easy_init() failed.");
		return;
	}

	nlohmann::json body;
	body["query"] = Query;
	std::string bodyStr = body.dump();

	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)bodyStr.size());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Callbacks::WriteToString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &m_Response);

	auto res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		std::println("[CGraphQLRequest] curl_easy_perform() failed: {}", curl_easy_strerror(res));

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
}

const std::string& CGraphQLRequest::GetResponse() const
{
	return m_Response;
}
