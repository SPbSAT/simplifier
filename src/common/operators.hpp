#pragma once

#include "src/common/csat_types.hpp"
#include "src/utility/converters.hpp"

#include <cassert>
#include <functional>
#include <numeric>
#include <string>
#include <unordered_map>


/** Namespace contains functions that evaluate different Operators. **/
namespace csat::op
{


/**
 * Simple versions of Operators that take a Two or One (e.g. Not) GateState arguments.
 */


/**
 * Represents function which is operator of any number of arguments up to 3.
 * If operator's arity is lesser than 3, then redundant args will not be used
 * in its implementation. It is made so to ease API of operator calling.
 */
using Operator = GateState(*)(GateState, GateState, GateState);


inline GateState NOT(GateState a, GateState=GateState::UNDEFINED, GateState=GateState::UNDEFINED) noexcept
{
    static GateState table_[GateStateNumber]
    {
        GateState::TRUE,        // GateState::FALSE
        GateState::FALSE,       // GateState::TRUE
        GateState::UNDEFINED,   // GateState::UNDEFINED
    };
    return table_[static_cast<uint8_t>(a)];
}

inline GateState AND(GateState a, GateState b, GateState=GateState::UNDEFINED) noexcept
{
    static GateState table_[GateStateNumber * GateStateNumber]
    {
        GateState::FALSE, GateState::FALSE,     GateState::FALSE,       // GateState::FALSE
        GateState::FALSE, GateState::TRUE,      GateState::UNDEFINED,   // GateState::TRUE
        GateState::FALSE, GateState::UNDEFINED, GateState::UNDEFINED,   // GateState::UNDEFINED
    };
    return table_[static_cast<uint8_t>(a) * GateStateNumber + static_cast<uint8_t>(b)];
}

inline GateState OR(GateState a, GateState b, GateState=GateState::UNDEFINED) noexcept
{
    static GateState table_[GateStateNumber * GateStateNumber]
    {
        GateState::FALSE,     GateState::TRUE, GateState::UNDEFINED,    // GateState::FALSE
        GateState::TRUE,      GateState::TRUE, GateState::TRUE,         // GateState::TRUE
        GateState::UNDEFINED, GateState::TRUE, GateState::UNDEFINED,    // GateState::UNDEFINED
    };
    return table_[static_cast<uint8_t>(a) * GateStateNumber + static_cast<uint8_t>(b)];
}

inline GateState XOR(GateState a, GateState b, GateState=GateState::UNDEFINED) noexcept
{
    static GateState table_[GateStateNumber * GateStateNumber]
    {
        GateState::FALSE,     GateState::TRUE,      GateState::UNDEFINED,   // GateState::FALSE
        GateState::TRUE,      GateState::FALSE,     GateState::UNDEFINED,   // GateState::TRUE
        GateState::UNDEFINED, GateState::UNDEFINED, GateState::UNDEFINED,   // GateState::UNDEFINED
    };
    return table_[static_cast<uint8_t>(a) * GateStateNumber + static_cast<uint8_t>(b)];
}

inline GateState NAND(GateState a, GateState b, GateState=GateState::UNDEFINED) noexcept
{
    static GateState table_[GateStateNumber * GateStateNumber]
    {
        GateState::TRUE, GateState::TRUE,      GateState::TRUE,         // GateState::FALSE
        GateState::TRUE, GateState::FALSE,     GateState::UNDEFINED,    // GateState::TRUE
        GateState::TRUE, GateState::UNDEFINED, GateState::UNDEFINED,    // GateState::UNDEFINED
    };
    return table_[static_cast<uint8_t>(a) * GateStateNumber + static_cast<uint8_t>(b)];
}

inline GateState NOR(GateState a, GateState b, GateState=GateState::UNDEFINED) noexcept
{
    static GateState table_[GateStateNumber * GateStateNumber]
    {
        GateState::TRUE,      GateState::FALSE, GateState::UNDEFINED,   // GateState::FALSE
        GateState::FALSE,     GateState::FALSE, GateState::FALSE,       // GateState::TRUE
        GateState::UNDEFINED, GateState::FALSE, GateState::UNDEFINED,   // GateState::UNDEFINED
    };
    return table_[static_cast<uint8_t>(a) * GateStateNumber + static_cast<uint8_t>(b)];
}

inline GateState NXOR(GateState a, GateState b, GateState=GateState::UNDEFINED) noexcept
{
    static GateState table_[GateStateNumber * GateStateNumber]
    {
        GateState::TRUE,      GateState::FALSE,     GateState::UNDEFINED,   // GateState::FALSE
        GateState::FALSE,     GateState::TRUE,      GateState::UNDEFINED,   // GateState::TRUE
        GateState::UNDEFINED, GateState::UNDEFINED, GateState::UNDEFINED,   // GateState::UNDEFINED
    };
    return table_[static_cast<uint8_t>(a) * GateStateNumber + static_cast<uint8_t>(b)];
}

inline GateState IFF(GateState a, GateState=GateState::UNDEFINED, GateState=GateState::UNDEFINED) noexcept
{
    static GateState table_[GateStateNumber]
        {
            GateState::FALSE,       // GateState::FALSE
            GateState::TRUE,        // GateState::TRUE
            GateState::UNDEFINED,   // GateState::UNDEFINED
        };
    return table_[static_cast<uint8_t>(a)];
}

inline GateState MUX(GateState x, GateState y, GateState z) noexcept
{
    static GateState table_[3 * GateStateNumber * GateStateNumber]
        {
            // x is FALSE, then we take value of y
            GateState::FALSE,     GateState::FALSE,     GateState::FALSE,       // GateState::FALSE
            GateState::TRUE,      GateState::TRUE,      GateState::TRUE,        // GateState::TRUE
            GateState::UNDEFINED, GateState::UNDEFINED, GateState::UNDEFINED,   // GateState::UNDEFINED
            // x is TRUE, then we take value of z
            GateState::FALSE,     GateState::TRUE,      GateState::UNDEFINED,   // GateState::FALSE
            GateState::FALSE,     GateState::TRUE,      GateState::UNDEFINED,   // GateState::TRUE
            GateState::FALSE,     GateState::TRUE,      GateState::UNDEFINED,   // GateState::UNDEFINED
            // x is UNDEFINED, then all answers are UNDEFINED
            GateState::UNDEFINED, GateState::UNDEFINED, GateState::UNDEFINED,   // GateState::FALSE
            GateState::UNDEFINED, GateState::UNDEFINED, GateState::UNDEFINED,   // GateState::TRUE
            GateState::UNDEFINED, GateState::UNDEFINED, GateState::UNDEFINED,   // GateState::UNDEFINED
        };
    return table_[
        0
        + static_cast<uint8_t>(x) * 9
        + static_cast<uint8_t>(y) * GateStateNumber
        + static_cast<uint8_t>(z)];
}

inline GateState CONST_FALSE(
    GateState=GateState::UNDEFINED,
    GateState=GateState::UNDEFINED,
    GateState=GateState::UNDEFINED) noexcept
{
    return GateState::FALSE;
}

inline GateState CONST_TRUE(
    GateState=GateState::UNDEFINED,
    GateState=GateState::UNDEFINED,
    GateState=GateState::UNDEFINED) noexcept
{
    return GateState::TRUE;
}


/**
 * @return: binary operator reference by GateType value.
 */
inline Operator getOperator(GateType type) noexcept
{
    assert(type != GateType::INPUT);
    assert(type != GateType::BUFF);
    assert(type != GateType::UNDEFINED);
    // Must be changed if csat::GateType is changed
    static Operator operators_[SupportedOperatorNumber]
    {
        &NOT,
        &AND, &NAND,
        &OR,  &NOR,
        &XOR, &NXOR,
        &IFF,
        &MUX,
        &CONST_FALSE,
        &CONST_TRUE
    };
    return operators_[getIndexByOperator(type)];
}


/**
 * FoldMap versions of Operators are used to evaluate Operators over some containers
 * (like a GateIdContainer) while knowing their assignment values in mapper.
 */
 

template<class T>
using ContainerT = std::vector<T>;
template<class T>
using MapFunction = std::function<GateState(T)>;
template<class T>
using OperatorNT = GateState(*)(ContainerT<T> const& container, MapFunction<T> mapper);


namespace
{

/**
 * Must be used only for binary operators.
 */
template<class T, GateState TerminalState=GateState::UNDEFINED>
inline GateState FoldMapOperator_(
    Operator oper,
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() >= 2) && "Can't foldMap container with less then 2 elements.");
    GateState state = oper(mapper(container.at(0)), mapper(container.at(1)), GateState::UNDEFINED);
    for(auto it = container.begin() + 2; it != container.end(); ++it)
    {
        if constexpr(TerminalState != GateState::UNDEFINED)
        {
            if (state == TerminalState)
            {
                return state;
            }
        }
        state = oper(state, mapper(*it), GateState::UNDEFINED);

    }
    return state;

}

} // anonymous namespace


template<class T>
inline GateState NOT(
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() == 1) && "Wrong number of arguments for NOT.");
    return NOT(mapper(container.at(0)));
}

template<class T>
inline GateState AND(
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() >= 2) && "Wrong number of arguments for AND.");
    return FoldMapOperator_<T, GateState::FALSE>(&AND, container, mapper);
}

template<class T>
inline GateState OR(
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() >= 2) && "Wrong number of arguments for OR.");
    return FoldMapOperator_<T, GateState::TRUE>(&OR, container, mapper);
}

template<class T>
inline GateState XOR(
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() >= 2) && "Wrong number of arguments for XOR.");
    return FoldMapOperator_<T>(&XOR, container, mapper);
}

template<class T>
inline GateState NAND(
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() >= 2) && "Wrong number of arguments for NAND.");
    return NOT(AND(container, mapper));
}

template<class T>
inline GateState NOR(
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() >= 2) && "Wrong number of arguments for NOR.");
    return NOT(OR(container, mapper));
}

template<class T>
inline GateState NXOR(
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() >= 2) && "Wrong number of arguments for NXOR.");
    return NOT(XOR(container, mapper));
}

template<class T>
inline GateState MUX(
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() == 3) && "Wrong number of arguments for MUX.");
    return MUX(mapper(container.at(0)), mapper(container.at(1)), mapper(container.at(2)));
}

template<class T>
inline GateState IFF(
    ContainerT<T> const& container,
    MapFunction<T> mapper) noexcept
{
    assert((container.size() == 1) && "Wrong number of arguments for IFF.");
    return IFF(mapper(container.at(0)));
}

template<class T>
inline GateState CONST_FALSE(
    [[maybe_unused]] ContainerT<T> const& container,
    [[maybe_unused]] MapFunction<T> mapper) noexcept
{
    assert((container.size() == 0) && "Wrong number of arguments for CONST_FALSE.");
    return GateState::FALSE;
}

template<class T>
inline GateState CONST_TRUE(
    [[maybe_unused]] ContainerT<T> const& container,
    [[maybe_unused]] MapFunction<T> mapper) noexcept
{
    assert((container.size() == 0) && "Wrong number of arguments for CONST_TRUE.");
    return GateState::TRUE;
}


/**
 * @return (ContainerT<T>, MapFunction<T>) argument operator reference by GateType value.
 */
template<class T>
inline OperatorNT<T> getOperatorNT(GateType type) noexcept
{
    assert(type != GateType::INPUT);
    assert(type != GateType::BUFF);
    assert(type != GateType::UNDEFINED);
    // Must be changed if csat::GateType is changed
    static OperatorNT<T> operators_[SupportedOperatorNumber]
    {
        &NOT<T>,
        &AND<T>, &NAND<T>,
        &OR<T>,  &NOR<T>,
        &XOR<T>, &NXOR<T>,
        &IFF<T>,
        &MUX<T>,
        &CONST_FALSE<T>,
        &CONST_TRUE<T>
    };
    return operators_[getIndexByOperator(type)];
}


} // csat::op namespace
