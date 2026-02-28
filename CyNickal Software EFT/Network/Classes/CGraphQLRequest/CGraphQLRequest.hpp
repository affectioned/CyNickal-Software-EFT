#pragma once

class CGraphQLRequest
{
public:
	CGraphQLRequest(const std::string& URL, const std::string& Query);
	const std::string& GetResponse() const;

private:
	std::string m_Response{};
};
