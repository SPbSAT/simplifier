#pragma once

#include "src/utility/logger.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
#include <set>


/**
 * Main header that contains some global constants and general types.
 */
namespace csat
{

// `vector<char>` is used because `vector<bool>` template
// specialization is memory but not time efficient.
using BoolVector = std::vector<char>;

/** Minimum arity of a logical function. **/
enum class MinArity : uint8_t
{
    NULLARY = 0,
    UNARY   = 1,
    BINARY  = 2,
    TERNARY = 3,
};

/** Universal return codes **/
enum class ReturnCode : uint8_t
{
    UNDEFINED = 0,
    SAT       = 10,
    UNSAT     = 20
};

/** Possible states of gates. **/
enum class GateState : uint8_t
{
  // if changed, then csat::op must be changed as well
  FALSE     = 0,
  TRUE      = 1,
  UNDEFINED = 2
};
/** Number of possible Gate States **/
constexpr size_t GateStateNumber = 3;


/** Possible types of gates. **/
enum class GateType : uint8_t
{
  // if changed, then csat::op must be changed as well
  INPUT  = 0,
  NOT    = 1,
  AND    = 2, NAND =  3,
  OR     = 4, NOR  =  5,
  XOR    = 6, NXOR =  7,
  IFF    = 8,
  MUX    = 9,
  // constant operator types
  CONST_FALSE = 10,
  CONST_TRUE = 11,
  // special operator types
  BUFF = UINT8_MAX - 1,
  UNDEFINED = UINT8_MAX
};
/** Number of supported Operator Gates **/
constexpr size_t SupportedOperatorNumber = 11;
/** Index of first Operator Gate in GateType enum **/
constexpr size_t FirstOperatorIdx = 1;


/**
 * @return index of gateType among all gate operator types.
 */
inline size_t getIndexByOperator(GateType gateType)
{
    return static_cast<size_t>(gateType) - FirstOperatorIdx;
}

/** Internal gate ids are numbers 0,1,2... **/
using GateId = size_t;
using GateIdContainer = std::vector<GateId>;

} // csat namespace
