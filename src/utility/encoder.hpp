#pragma once

#include "src/common/csat_types.hpp"

#include <memory>
#include <map>


namespace csat::utils
{


template<class KeyT>
class GateEncoder
{
  protected:
    /* Next var to be added to encoding map. */
    size_t next_var_ = 0;
    /* Encoding map `variable name`->`numerical value`. */
    std::map<KeyT, GateId> encoder_{};
    /* Inverse encoding map. */
    std::map<GateId, KeyT> decoder_{};
   
  public:
    GateEncoder() = default;
    GateEncoder(GateEncoder const&) = default;
    ~GateEncoder() = default;
    
    /**
     * @param key -- name of a gate that need to be encoded.
     * @return id of gate in encoding from 0 through N.
     */
    GateId encodeGate(KeyT const& key)
    noexcept
    {
        auto search = encoder_.find(key);
        if (search == encoder_.end())
        {
            decoder_[next_var_] = key;
            encoder_[key] = next_var_;
            return next_var_++;
        }
        else
        {
            return search->second;
        }
    };
    
    /**
     * @param id -- id of gate in encoding from 0 through N.
     * @return Original gate name (key).
     */
    [[nodiscard]]
    KeyT decodeGate(GateId id) const
    {
        return decoder_.at(id);
    };
  
    /**
     * @param key -- name of a gate that need to be check for presence.
     * @return True if key is in class otherwise False.
     */
    [[nodiscard]]
    bool keyExists(KeyT const& key) const
    {
        return encoder_.find(key) != encoder_.end();
    };
  
    /**
     * Returns number of elements in encoding.
     */
     [[nodiscard]]
     size_t size() const
     {
         return next_var_;
     }
    
    /**
     * Clears internal state of encoder.
     */
    void clear()
    {
        next_var_ = 0;
        encoder_.clear();
        decoder_.clear();
    }
};


/**
 * GateEncoder specification for string, that allows usage of string_view.
 */
template<>
class GateEncoder<std::string>
{
  protected:
    size_t next_var_ = 0;
    std::map<std::string, GateId, std::less<>> encoder_{};
    std::map<GateId, std::string> decoder_{};
   
  public:
    GateEncoder() = default;
    GateEncoder(GateEncoder const&) = default;
    ~GateEncoder() = default;
    
    GateId encodeGate(std::string_view const& key)
    noexcept
    {
        auto search = encoder_.find(key);
        if (search == encoder_.end())
        {
            // TODO: rly string???
            decoder_[next_var_] = std::string(key);
            encoder_[std::string(key)] = next_var_;
            return next_var_++;
        }
        else
        {
            return search->second;
        }
    };
  
    [[nodiscard]]
    std::string decodeGate(GateId id) const
    {
        return decoder_.at(id);
    };
  
    [[nodiscard]]
    bool keyExists(std::string_view const& key) const
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


template<class KeyT>
inline std::unique_ptr<GateEncoder<KeyT>> mergeGateEncoders(
    GateEncoder<KeyT> const& first,
    GateEncoder<GateId> const& second)
noexcept
{
    GateEncoder<KeyT> _newEncoder;

    for (size_t code_two = 0; code_two < second.size(); ++code_two)
    {
        _newEncoder.encodeGate(first.decodeGate(second.decodeGate(code_two)));
    }
    
    return std::make_unique<GateEncoder<KeyT>>(_newEncoder);
}


} // csat::utils namespace
