#pragma once

#include "src/simplification/transformer_base.hpp"

#include <type_traits>
#include <memory>


namespace csat::simplification
{


/**
 * Class that represents a composition of transformers.
 *
 * @tparam CircuitT -- class that carries circuit.
 * @tparam TransformerT -- first transformer to be applied in composition.
 * @tparam OtherTransformersT -- other transformers in composition.
 */
template<
    class CircuitT,
    class TransformerT,
    class ...OtherTransformersT
>
struct Composition : public ITransformer<CircuitT>
{
    static_assert(
        std::is_base_of<ICircuit, CircuitT>::value,
        "CircuitT must be implementation of an ICircuit."
    );
    static_assert(
        (
            std::is_base_of_v<
                ITransformer<CircuitT>, OtherTransformersT
            > && ...
        ),
        "All Preprocessor template args of Composition must implement "
        "IPreprocessor and be parametrized with CircuitT type."
    );
 
  public:
    /**
     * Applies all defined in template TransformersT to
     * `circuit`, in left-to-right order. For example,
     * Composition<DAG, T1, T2, T3>::apply(circuit) will perform
     *
     *     T3().transform(T2().transform(T1().transform(circuit)))
     *
     * @param circuit -- circuit to transform.
     * @return circuit, that is result of transformation.
     */
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<GateEncoder<std::string>> encoder)
    {
        auto _transformer = TransformerT();
        auto [_circuit, _encoder] = _transformer.transform(
            std::move(circuit),
            std::move(encoder));
        
        Composition<CircuitT, OtherTransformersT...> obj_composition;
        return obj_composition.transform(
            std::move(_circuit),
            std::move(_encoder)
        );
    }
};


/**
 * Composition specification for one Transformer.
 *
 * @tparam CircuitT -- class that carries circuit.
 * @tparam TransformerT -- first transformer to be applied in composition.
 */
template<
    class CircuitT,
    class TransformerT
>
struct Composition<CircuitT, TransformerT> : public ITransformer<CircuitT>
{
  public:
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<GateEncoder<std::string>> encoder)
    {
        auto _transformer = TransformerT();
        return _transformer.transform(
            std::move(circuit),
            std::move(encoder));
    }
};

} // csat namespace
