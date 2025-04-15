#include "libinfisical/InfisicalClient.h"

namespace Infisical
{

  Infisical::ConfigBuilder &ConfigBuilder::withHostUrl(std::string url)
  {
    config_.url_ = url;
    return *this;
  }

  Infisical::ConfigBuilder &ConfigBuilder::withAuthentication(Authentication &&auth)
  {
    config_.authentication_ = std::move(auth);
    return *this;
  }

  Infisical::Config &ConfigBuilder::build()
  {

    if (config_.url_.empty())
    {
      throw std::invalid_argument("Config URL cannot be empty");
    }

    if (config_.url_.size() >= 4 && config_.url_.substr(config_.url_.size() - 4) == "/api")
    {
      config_.url_ = config_.url_.substr(0, config_.url_.size() - 4);
    }

    if (!config_.url_.empty() && config_.url_.back() == '/')
    {
      config_.url_ = config_.url_.substr(0, config_.url_.size() - 1);
    }

    return config_;
  }

}
