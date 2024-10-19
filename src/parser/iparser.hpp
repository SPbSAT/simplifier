#pragma once

#include <iostream>

/**
 * Parser from CircuitSAT.BENCH file (stream of lines) to structure that carries Circuit.
 */
namespace csat::parser
{

/**
 * Base class for a boolean circuit parsers.
 */
class ICircuitParser
{
  public:
    virtual ~ICircuitParser() = default;
    
    /**
     * Clears internal state of a parser.
     */
    virtual void clear() = 0;
    
    /**
     * Parser info from stream.
     * @param stream -- text stream, containing lines of some .BENCH file.
     */
    virtual void parseStream(std::istream& stream) = 0;
};

} // namespace csat::parser
