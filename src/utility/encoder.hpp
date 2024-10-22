#pragma once

#include <map>
#include <memory>
#include <string_view>

#include "src/common/csat_types.hpp"

namespace csat::utils
{

/**
 * GateEncoder which allows to map original gate names to contiguous integers range.
 */
class GateEncoder
{
  protected:
    size_t next_var_ = 0;
    std::map<std::string, GateId, std::less<>> encoder_{};
    std::map<GateId, std::string> decoder_{};

  public:
    GateEncoder()                   = default;
    GateEncoder(GateEncoder const&) = default;
    ~GateEncoder()                  = default;

    GateId encodeGate(std::string_view key) noexcept
    {
        auto search = encoder_.find(key);
        if (search == encoder_.end())
        {
            decoder_[next_var_]        = std::string(key);
            encoder_[std::string(key)] = next_var_;
            return next_var_++;
        }
        else
        {
            return search->second;
        }
    };

    [[nodiscard]]
    std::string const& decodeGate(GateId id) const
    {
        return decoder_.at(id);
    };

    [[nodiscard]]
    bool keyExists(std::string_view key) const
    {
        return encoder_.find(key) != encoder_.end();
    };

    [[nodiscard]]
    size_t size() const
    {
        return next_var_;
    }

    void clear()
    {
        next_var_ = 0;
        encoder_.clear();
        decoder_.clear();
    }
};

}  // namespace csat::utils
