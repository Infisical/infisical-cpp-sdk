#include "libinfisical/InfisicalClient.h"
#include <stdexcept>
#include <iostream>
#include <stdio.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

std::string httpMethodStringRepresentation(Infisical::http::Method method)
{
  switch (method)
  {
  case Infisical::http::Method::GET:
    return "GET";
  case Infisical::http::Method::POST:
    return "POST";
  case Infisical::http::Method::PATCH:
    return "PATCH";
  case Infisical::http::Method::DELETE:
    return "DELETE";
  default:
    throw std::invalid_argument("Invalid HTTP method");
  }
}

namespace Infisical
{

  namespace http
  {

    HttpClient::HttpClient() : m_timeout(30000)
    {
      // Set some sensible defaults
      m_defaultHeaders["User-Agent"] = "infisical-cpp-sdk";
      m_defaultHeaders["Accept"] = "application/json";
      m_defaultHeaders["Content-Type"] = "application/json";
    }

    HttpClient::HttpClient(const std::string &baseUrl) : HttpClient()
    {
      m_baseUrl = baseUrl;
    }

    void HttpClient::setBaseUrl(const std::string &baseUrl)
    {
      m_baseUrl = baseUrl;
    }

    void HttpClient::setDefaultHeader(const std::string &name, const std::string &value)
    {
      m_defaultHeaders[name] = value;
    }

    cpr::Header HttpClient::mergeHeaders(const std::map<std::string, std::string> &headers)
    {
      cpr::Header merged;

      // Add default headers
      for (const auto &[key, value] : m_defaultHeaders)
      {
        merged[key] = value;
      }

      // Add (or override) with request-specific headers
      for (const auto &[key, value] : headers)
      {
        merged[key] = value;
      }

      return merged;
    }

    cpr::Parameters HttpClient::convertParams(const std::map<std::string, std::string> &params)
    {
      cpr::Parameters cprParams;

      for (const auto &[key, value] : params)
      {
        cprParams.Add({key, value});
      }

      return cprParams;
    }

    cpr::Response HttpClient::request(
        Method method,
        const std::string &endpoint,
        const std::map<std::string, std::string> &headers,
        const std::map<std::string, std::string> &params,
        const std::string &body)
    {
      // Prepare the URL
      std::string url = m_baseUrl + endpoint;

      // Prepare headers and parameters
      cpr::Header mergedHeaders = mergeHeaders(headers);
      cpr::Parameters cprParams = convertParams(params);

      // Create a session
      cpr::Session session;
      session.SetUrl(url);
      session.SetHeader(mergedHeaders);
      session.SetParameters(cprParams);
      session.SetTimeout(m_timeout);

      // Set body for appropriate methods
      if (!body.empty() && (method == Method::POST || method == Method::PATCH || method == Method::DELETE))
      {
        session.SetBody(body);
      }

      // Execute the request based on the method
      cpr::Response response;

      switch (method)
      {
      case Method::GET:
        response = session.Get();
        break;
      case Method::POST:
        response = session.Post();
        break;
      case Method::PATCH:
        response = session.Patch();
        break;
      case Method::DELETE:
        response = session.Delete();
        break;
      default:
        throw std::invalid_argument("Invalid HTTP method");
      }

      // note(daniel): should probably also check for status code = 0 here, because status code will be 0 if there was a network error
      if (response.error)
      {
        throw Infisical::InfisicalError("Network error: " + response.error.message, 0, "");
      }

      std::string errorMsg = "";
      if (response.status_code < 200 || response.status_code >= 400)
      {
        nlohmann::json jsonResponse;
        try
        {
          jsonResponse = nlohmann::json::parse(response.text);

          std::string errorMessageStr;

          // will always contain request ID if it contains a message
          if (jsonResponse.contains("message"))
          {

            std::string reqId = jsonResponse.contains("reqId") ? jsonResponse["reqId"].get<std::string>() : "Unknown";
            errorMessageStr =
                jsonResponse["message"].is_string()                                           ? jsonResponse["message"].get<std::string>()
                : (jsonResponse["message"].is_array() || jsonResponse["message"].is_object()) ? jsonResponse["message"].dump()
                                                                                              : "Unknown error format";
            int requiredBufferSize = snprintf(
                nullptr,
                0,
                "HTTP Error: [url=%s] [method=%s] [status-code=%ld] [request-id=%s] [message=%s]",
                url.c_str(),
                httpMethodStringRepresentation(method).c_str(),
                response.status_code,
                reqId.c_str(),
                errorMessageStr.c_str());

            std::vector<char> buffer(requiredBufferSize + 1);
            snprintf(
                buffer.data(),
                buffer.size(),
                "HTTP Error: [url=%s] [method=%s] [status-code=%ld] [request-id=%s] [message=%s]",
                url.c_str(),
                httpMethodStringRepresentation(method).c_str(),
                response.status_code,
                reqId.c_str(),
                errorMessageStr.c_str());

            std::string msg(buffer.data(), buffer.size() - 1);
            throw Infisical::InfisicalError(msg, response.status_code, response.text);
          }
        }
        catch (const nlohmann::json::exception &e)
        {

          int requiredBufferSize = snprintf(
              nullptr,
              0,
              "HTTP Error: [url=%s] [method=%s] [status-code=%ld]",
              url.c_str(),
              httpMethodStringRepresentation(method).c_str(),
              response.status_code);

          std::vector<char> buffer(requiredBufferSize + 1);

          snprintf(
              buffer.data(),
              buffer.size(),
              "HTTP Error: [url=%s] [method=%s] [status-code=%ld]",
              url.c_str(),
              httpMethodStringRepresentation(method).c_str(),
              response.status_code);

          errorMsg = std::string(buffer.data(), buffer.size() - 1);
        }
        catch (const Infisical::InfisicalError &)
        {
          // rethrow the infisical error
          throw;
        }
        catch (...)
        {

          int requiredBufferSize = snprintf(
              nullptr,
              0,
              "HTTP Error: [url=%s] [method=%s] [status-code=%ld]",
              url.c_str(),
              httpMethodStringRepresentation(method).c_str(),
              response.status_code);

          std::vector<char> buffer(requiredBufferSize + 1);

          snprintf(
              buffer.data(),
              buffer.size(),
              "HTTP Error: [url=%s] [method=%s] [status-code=%ld]",
              url.c_str(),
              httpMethodStringRepresentation(method).c_str(),
              response.status_code);

          errorMsg = std::string(buffer.data(), buffer.size() - 1);
        }

        throw Infisical::InfisicalError(errorMsg, response.status_code, response.text);
      }

      return response;
    }

    cpr::Response HttpClient::get(
        const std::string &endpoint,
        const std::map<std::string, std::string> &headers,
        const std::map<std::string, std::string> &params)
    {
      return request(Method::GET, endpoint, headers, params);
    }

    cpr::Response HttpClient::post(
        const std::string &endpoint,
        const std::map<std::string, std::string> &headers,
        const std::string &body)
    {
      return request(Method::POST, endpoint, headers, {}, body);
    }

    cpr::Response HttpClient::patch(
        const std::string &endpoint,
        const std::map<std::string, std::string> &headers,
        const std::string &body)
    {
      return request(Method::PATCH, endpoint, headers, {}, body);
    }

    cpr::Response HttpClient::del(
        const std::string &endpoint,
        const std::map<std::string, std::string> &headers,
        const std::string &body)
    {
      return request(Method::DELETE, endpoint, headers, {}, body);
    }

  } // namespace http
}