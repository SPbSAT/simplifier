#pragma once

#include "src/structures/circuit/icircuit.hpp"

#include "src/utility/logger.hpp"
#include "src/common/csat_types.hpp"
#include "src/common/operators.hpp"

#include "src/structures/assignment/vector_assignment.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>


namespace csat
{

/** Auxiliary structure to represent Gate as Node in DAG. **/
struct Node_
{
  private:
    /* Gate number (id). */
    GateId id_ = 0;
    /* Gate type. */
    GateType type_ = GateType::UNDEFINED;
    /* Gate operands container. */
    GateIdContainer operands_;
    /* Users of gate -- gates, that have current gate as operand. */
    GateIdContainer users_;
  
  public:
    Node_() = default;
    ~Node_() = default;
    Node_(Node_& node) = default;
    Node_(Node_ const& node) = default;
    Node_& operator=(Node_ const& node) = default;
    
    Node_(Node_&& node) noexcept
        : id_(std::exchange(node.id_, 0))
        , type_(std::exchange(node.type_, GateType::UNDEFINED))
        , operands_(std::exchange(node.operands_, {}))
        , users_(std::exchange(node.users_, {})) {};
    
    Node_(GateId gateId, GateType type, GateIdContainer const& operands, GateIdContainer const& users) noexcept
        : id_(gateId)
        , type_(type)
        , operands_{operands}
        , users_({users}) {}
    
    Node_(GateId gateId, GateType type, GateIdContainer&& operands, GateIdContainer&& users) noexcept
        : id_(gateId)
        , type_(type)
        , operands_{std::move(operands)}
        , users_({std::move(users)}) {}
    
    Node_(GateId gateId, GateType type, GateIdContainer const& operands) noexcept
        : id_(gateId) , type_(type) , operands_{operands} {}
    
    Node_(GateId gateId, GateType type, GateIdContainer&& operands) noexcept
        : id_(gateId) , type_(type) , operands_{std::move(operands)} {}
    
    [[maybe_unused, nodiscard]]
    GateId getId() const noexcept
    {
        return id_;
    }
    
    [[maybe_unused, nodiscard]]
    GateType getType() const noexcept
    {
        return type_;
    }
    
    [[maybe_unused, nodiscard]]
    GateIdContainer const& getOperands() const noexcept
    {
        return operands_;
    }
    
    [[maybe_unused, nodiscard]]
    GateIdContainer const& getGateUsers() const noexcept
    {
        return users_;
    }
    
    [[maybe_unused]]
    void addUser(GateId gateId)
    {
        users_.push_back(gateId);
    }
};


/** Represents boolean circuit as Directed Acyclic Graph. **/
class DAG : public ICircuit
{
  protected:
    /* Carries all gates. */
    std::vector<Node_> gates_;
    /* Carries all input gates. */
    GateIdContainer input_gates_;
    /* Carries all output gates.. */
    GateIdContainer output_gates_;
    
  public:
    DAG(DAG const& dag)
        : gates_(dag.gates_)
        , input_gates_(dag.input_gates_)
        , output_gates_(dag.output_gates_) {}
    
    DAG(GateInfoContainer const& gate_info, GateIdContainer const& output_gates)
        : output_gates_(output_gates)
    {
        comprehendGateInfo_(gate_info);
    }
    
    DAG(GateInfoContainer&& gate_info, GateIdContainer&& output_gates)
        : output_gates_(std::move(output_gates))
    {
        comprehendGateInfo_(std::move(gate_info));
    }
    
    ~DAG() override = default;
  
  private:
    template<class T>
    void comprehendGateInfo_(T&& gate_info)
    {
        buildGates_(std::forward<T>(gate_info));
        calculateGateUsers_();
    }
    
    void buildGates_(GateInfoContainer const& gate_info)
    {
        gates_.reserve(gate_info.size());
        for (size_t gateId = 0; gateId < gate_info.size(); ++gateId)
        {
            gates_.emplace_back(
                gateId,
                gate_info[gateId].getType(),
                gate_info[gateId].getOperands());
        
            if (gate_info[gateId].getType() == GateType::INPUT)
            {
                input_gates_.push_back(gateId);
            }
        }
    }
    
    void buildGates_(GateInfoContainer&& gate_info)
    {
        gates_.reserve(gate_info.size());
        for (size_t gateId = 0; gateId < gate_info.size(); ++gateId)
        {
            gates_.emplace_back(
                gateId,
                gate_info[gateId].getType(),
                gate_info[gateId].moveOperands());
            
            if (gate_info[gateId].getType() == GateType::INPUT)
            {
                input_gates_.push_back(gateId);
            }
        }
    }
    
    void calculateGateUsers_()
    {
        for (auto& gate : gates_)
        {
            for (auto operand : gate.getOperands())
            {
                // `at` here is used to implicitly check that
                // any operand is contained in this graph.
                // This check is ok, since it is doesn't create
                // sensible overhead for whole framework.
                gates_.at(operand).addUser(gate.getId());
            }
        }
    }
    
  public:
    /**
     * @return Number of gates in Circuit instance.
     */
    [[nodiscard]]
    GateId getNumberOfGates() const noexcept override
    {
        return gates_.size();
    };
  
    /**
     * @return Container with all Output gates.
     */
    [[nodiscard]]
    GateIdContainer const& getOutputGates() const noexcept override
    {
        return output_gates_;
    };
  
    /**
     * @return Container with all Input gates.
     */
    [[nodiscard]]
    GateIdContainer const& getInputGates() const noexcept override
    {
        return input_gates_;
    };
    
    /**
     * @param gateId
     * @return true iff gateId is output.
     */
    [[nodiscard]] bool isOutputGate(GateId gateId) const noexcept override
    {
        return std::find(
            output_gates_.begin(),
            output_gates_.end(),
            gateId) != output_gates_.end();
    };
  
    /**
     * @param gateId -- gate id.
     * @return type of gate with id=gateId.
     */
    [[nodiscard]]
    GateType getGateType(GateId gateId) const override
    {
        return getGate_(gateId).getType();
    };
  
    /**
     * @param gateId -- gate id.
     * @return Container with all operands (gate ids) of gate with id=gateId.
     */
    [[nodiscard]]
    GateIdContainer const& getGateOperands(GateId gateId) const override
    {
        return getGate_(gateId).getOperands();
    };
  
    /**
     * @param gateId -- gate id.
     * @return Container with all gates, that use gate with id=gateId as operand.
     */
    [[nodiscard]]
    GateIdContainer const& getGateUsers(GateId gateId) const override
    {
        return getGate_(gateId).getGateUsers();
    };
  
  protected:
    /* Returns reference to Node_. */
    [[nodiscard]]
    Node_ const& getGate_(GateId gateId) const
    {
        // We can't use member function `at` here, because this member function
        // is implicitly used in other member functions (e.g. getGateOperands,
        // getGateType, getGateUsers) which are heavily used in the main algorithm
        // and their performance is important.
        return gates_[gateId];
    };
};


} // namespace csat
